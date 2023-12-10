///////////////////////////////////////////////////////////
//  A simple WebGL program to show how to load JSON model
//

var gl;
var canvas;
var matrixStack = [];

var zAngle = 0.0;
var yAngle = 0.0;

var prevMouseX = 0;
var prevMouseY = 0;
var aPositionLocation;
var uVMatrixLocation;
var uMMatrixLocation;
var uPMatrixLocation;

var objVertexPositionBuffer;
var objVertexNormalBuffer;
var objVertexIndexBuffer;

var uDiffuseTermLocation;

var vMatrix = mat4.create(); // view matrix
var mMatrix = mat4.create(); // model matrix
var pMatrix = mat4.create(); //projection matrix

var eyePos = [0.0, 2.0, 3.5];

// Inpur JSON model file to load
input_JSON = "teapot.json";

////////////////////////////////////////////////////////////////////
const vertexShaderCode = `#version 300 es
in vec3 aPosition;

uniform mat4 uMMatrix;
uniform mat4 uPMatrix;
uniform mat4 uVMatrix;

void main() {
  mat4 projectionModelView;
	projectionModelView=uPMatrix*uVMatrix*uMMatrix;
  // calculate clip space position
  gl_Position =  projectionModelView * vec4(aPosition,1.0);
  gl_PointSize=3.0;
}`;

const fragShaderCode = `#version 300 es
precision mediump float;

out vec4 fragColor;
uniform vec4 diffuseTerm;

void main() {
  fragColor = diffuseTerm;
}`;

function vertexShaderSetup(vertexShaderCode) {
  shader = gl.createShader(gl.VERTEX_SHADER);
  gl.shaderSource(shader, vertexShaderCode);
  gl.compileShader(shader);
  // Error check whether the shader is compiled correctly
  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    alert(gl.getShaderInfoLog(shader));
    return null;
  }
  return shader;
}

function fragmentShaderSetup(fragShaderCode) {
  shader = gl.createShader(gl.FRAGMENT_SHADER);
  gl.shaderSource(shader, fragShaderCode);
  gl.compileShader(shader);
  // Error check whether the shader is compiled correctly
  if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
    alert(gl.getShaderInfoLog(shader));
    return null;
  }
  return shader;
}

function initShaders() {
  shaderProgram = gl.createProgram();

  var vertexShader = vertexShaderSetup(vertexShaderCode);
  var fragmentShader = fragmentShaderSetup(fragShaderCode);

  // attach the shaders
  gl.attachShader(shaderProgram, vertexShader);
  gl.attachShader(shaderProgram, fragmentShader);
  //link the shader program
  gl.linkProgram(shaderProgram);

  // check for compilation and linking status
  if (!gl.getProgramParameter(shaderProgram, gl.LINK_STATUS)) {
    console.log(gl.getShaderInfoLog(vertexShader));
    console.log(gl.getShaderInfoLog(fragmentShader));
  }

  //finally use the program.
  gl.useProgram(shaderProgram);

  return shaderProgram;
}

function initGL(canvas) {
  try {
    gl = canvas.getContext("webgl2"); // the graphics webgl2 context
    gl.viewportWidth = canvas.width; // the width of the canvas
    gl.viewportHeight = canvas.height; // the height
  } catch (e) {}
  if (!gl) {
    alert("WebGL initialization failed");
  }
}

function degToRad(degrees) {
  return (degrees * Math.PI) / 180;
}

function pushMatrix(stack, m) {
  //necessary because javascript only does shallow push
  var copy = mat4.create(m);
  stack.push(copy);
}

function popMatrix(stack) {
  if (stack.length > 0) return stack.pop();
  else console.log("stack has no matrix to pop!");
}

function initObject() {
  // XMLHttpRequest objects are used to interact with servers
  // It can be used to retrieve any type of data, not just XML.
  var request = new XMLHttpRequest();
  request.open("GET", input_JSON);
  // MIME: Multipurpose Internet Mail Extensions
  // It lets users exchange different kinds of data files
  request.overrideMimeType("application/json");
  request.onreadystatechange = function () {
    //request.readyState == 4 means operation is done
    if (request.readyState == 4) {
      processObject(JSON.parse(request.responseText));
    }
  };
  request.send();
}

function processObject(objData) {
  objVertexPositionBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, objVertexPositionBuffer);
  gl.bufferData(
    gl.ARRAY_BUFFER,
    new Float32Array(objData.vertexPositions),
    gl.STATIC_DRAW
  );
  objVertexPositionBuffer.itemSize = 3;
  objVertexPositionBuffer.numItems = objData.vertexPositions.length / 3;

  objVertexIndexBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, objVertexIndexBuffer);
  gl.bufferData(
    gl.ELEMENT_ARRAY_BUFFER,
    new Uint32Array(objData.indices),
    gl.STATIC_DRAW
  );
  objVertexIndexBuffer.itemSize = 1;
  objVertexIndexBuffer.numItems = objData.indices.length;

  drawScene();
}

function drawObject(color) {
  gl.bindBuffer(gl.ARRAY_BUFFER, objVertexPositionBuffer);
  gl.vertexAttribPointer(
    aPositionLocation,
    objVertexPositionBuffer.itemSize,
    gl.FLOAT,
    false,
    0,
    0
  );

  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, objVertexIndexBuffer);

  gl.uniform4fv(uDiffuseTermLocation, color);
  gl.uniformMatrix4fv(uMMatrixLocation, false, mMatrix);
  gl.uniformMatrix4fv(uVMatrixLocation, false, vMatrix);
  gl.uniformMatrix4fv(uPMatrixLocation, false, pMatrix);

  gl.drawElements(
    gl.TRIANGLES,
    objVertexIndexBuffer.numItems,
    gl.UNSIGNED_INT,
    0
  );
}

//////////////////////////////////////////////////////////////////////
//The main drawing routine
function drawScene() {
  gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);
  gl.clearColor(0.9, 0.9, 0.9, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

  //set up the model matrix
  mat4.identity(mMatrix);

  // set up the view matrix, multiply into the modelview matrix
  mat4.identity(vMatrix);
  vMatrix = mat4.lookAt(eyePos, [0, 0, 0], [0, 1, 0], vMatrix);

  //set up projection matrix
  mat4.identity(pMatrix);
  mat4.perspective(60, 1.0, 0.01, 1000, pMatrix);

  mMatrix = mat4.rotate(mMatrix, degToRad(yAngle), [1, 0, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(zAngle), [0, 1, 0]);

  //draw teapot
  pushMatrix(matrixStack, mMatrix);
  color = [0.7, 0.2, 0.2, 1.0];
  mMatrix = mat4.scale(mMatrix, [0.12, 0.12, 0.12]);
  drawObject(color);
  mMatrix = popMatrix(matrixStack);
}

function onMouseDown(event) {
  document.addEventListener("mousemove", onMouseMove, false);
  document.addEventListener("mouseup", onMouseUp, false);
  document.addEventListener("mouseout", onMouseOut, false);

  if (
    event.layerX <= canvas.width &&
    event.layerX >= 0 &&
    event.layerY <= canvas.height &&
    event.layerY >= 0
  ) {
    prevMouseX = event.clientX;
    prevMouseY = canvas.height - event.clientY;
  }
}

function onMouseMove(event) {
  // make mouse interaction only within canvas
  if (
    event.layerX <= canvas.width &&
    event.layerX >= 0 &&
    event.layerY <= canvas.height &&
    event.layerY >= 0
  ) {
    var mouseX = event.clientX;
    var diffX = mouseX - prevMouseX;
    zAngle = zAngle + diffX / 5;
    prevMouseX = mouseX;

    var mouseY = canvas.height - event.clientY;
    var diffY = mouseY - prevMouseY;
    yAngle = yAngle - diffY / 5;
    prevMouseY = mouseY;

    drawScene();
  }
}

function onMouseUp(event) {
  document.removeEventListener("mousemove", onMouseMove, false);
  document.removeEventListener("mouseup", onMouseUp, false);
  document.removeEventListener("mouseout", onMouseOut, false);
}

function onMouseOut(event) {
  document.removeEventListener("mousemove", onMouseMove, false);
  document.removeEventListener("mouseup", onMouseUp, false);
  document.removeEventListener("mouseout", onMouseOut, false);
}

// This is the entry point from the html
function webGLStart() {
  canvas = document.getElementById("simpleLoadObjMesh");
  document.addEventListener("mousedown", onMouseDown, false);

  initGL(canvas);
  shaderProgram = initShaders();

  gl.enable(gl.DEPTH_TEST);

  //get locations of attributes declared in the vertex shader
  aPositionLocation = gl.getAttribLocation(shaderProgram, "aPosition");
  uMMatrixLocation = gl.getUniformLocation(shaderProgram, "uMMatrix");
  uPMatrixLocation = gl.getUniformLocation(shaderProgram, "uPMatrix");
  uVMatrixLocation = gl.getUniformLocation(shaderProgram, "uVMatrix");
  uDiffuseTermLocation = gl.getUniformLocation(shaderProgram, "diffuseTerm");

  //enable the attribute arrays
  gl.enableVertexAttribArray(aPositionLocation);

  //initialize buffers for the square
  initObject();
}
