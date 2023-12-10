//////////////////////////////////////////////////////////////////////////////////
//  A simple WebGL program to demonstration simple 2D hierarchical transformation
//

var gl;
var canvas;
var aPositionLocation;
var uMMatrixLocation;
var uColorLocation;
var sqVertexPositionBuffer;
var sqVertexIndexBuffer;
var prevMouseX = 0;
var prevMouseY = 0;
var zAngle = 0.0;
var degree0 = 0.0;
var degree1 = 0.0;
var degree2 = 0.0;
var model = mat4.create();
var matrixStack = [];

const vertexShaderCode = `#version 300 es
in vec2 aPosition;
uniform mat4 uMMatrix;

void main() {
  gl_Position = uMMatrix*vec4(aPosition,0.0,1.0);
}`;

const fragShaderCode = `#version 300 es
precision mediump float;
out vec4 fragColor;
uniform vec4 uColor;

void main() {
  fragColor = uColor;
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

function pushMatrix(stack, m) {
  //necessary because javascript only does shallow push
  var copy = mat4.create(m);
  stack.push(copy);
}

function popMatrix(stack) {
  if (stack.length > 0) return stack.pop();
  else console.log("stack has no matrix to pop!");
}

function degToRad(degrees) {
  return (degrees * Math.PI) / 180;
}

function initSquareBuffer() {
  // buffer for point locations
  const sqVertices = new Float32Array([
    0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5,
  ]);
  sqVertexPositionBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, sqVertexPositionBuffer);
  gl.bufferData(gl.ARRAY_BUFFER, sqVertices, gl.STATIC_DRAW);
  sqVertexPositionBuffer.itemSize = 2;
  sqVertexPositionBuffer.numItems = 4;

  // buffer for point indices
  const sqIndices = new Uint16Array([0, 1, 2, 0, 2, 3]);
  sqVertexIndexBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, sqVertexIndexBuffer);
  gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, sqIndices, gl.STATIC_DRAW);
  sqVertexIndexBuffer.itemsize = 1;
  sqVertexIndexBuffer.numItems = 6;
}

function drawSquare(mMatrix, color) {
  gl.uniformMatrix4fv(uMMatrixLocation, false, mMatrix);

  // buffer for point locations
  gl.bindBuffer(gl.ARRAY_BUFFER, sqVertexPositionBuffer);
  gl.vertexAttribPointer(
    aPositionLocation,
    sqVertexPositionBuffer.itemSize,
    gl.FLOAT,
    false,
    0,
    0
  );

  // buffer for point indices
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, sqVertexIndexBuffer);

  gl.uniform4fv(uColorLocation, color);

  // now draw the square
  gl.drawElements(
    gl.TRIANGLES,
    sqVertexIndexBuffer.numItems,
    gl.UNSIGNED_SHORT,
    0
  );
}

////////////////////////////////////////////////////////////////////////
// The main drawing routine
//
function drawScene() {
  gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);
  gl.clearColor(0.9, 0.9, 0.8, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

  // set the model matrix to identity first
  mat4.identity(model);

  // global rotation along z-axis, controlled by mouse
  model = mat4.rotate(model, degToRad(zAngle), [0, 0, 1]);
  // global rotation along z-axis, controlled by key press
  model = mat4.rotate(model, degToRad(degree2), [0, 0, 1]);

  // draw first square
  color = [1.0, 0.0, 0.0, 1.0];
  pushMatrix(matrixStack, model);
  model = mat4.scale(model, [0.25, 0.25, 0.25]);
  drawSquare(model, color);
  model = popMatrix(matrixStack);

  // pushMatrix(matrixStack, model);
  // draw first arm segmemnt
  model = mat4.translate(model, [0.0, -0.12, 0]);
  model = mat4.rotate(model, degToRad(degree0), [0, 0, 1]);
  pushMatrix(matrixStack, model);
  model = mat4.translate(model, [0.0, -0.2, 0]);
  model = mat4.scale(model, [0.05, 0.4, 1]);
  color = [0.0, 0.0, 1.0, 1.0];
  drawSquare(model, color);
  model = popMatrix(matrixStack);

  // model = popMatrix(matrixStack);
  // draw second arm segmemnt
  model = mat4.translate(model, [0, -0.4, 0]);
  model = mat4.rotate(model, degToRad(degree1), [0, 0, 1]);
  pushMatrix(matrixStack, model);
  model = mat4.translate(model, [0.2, 0.0, 0]);
  model = mat4.scale(model, [0.45, 0.05, 1]);
  color = [0.0, 1.0, 0.0, 1.0];
  drawSquare(model, color);
  model = popMatrix(matrixStack);
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

function onKeyPress(event) {
  var keyCode = event.keyCode;

  switch (keyCode) {
    case 82: //'r'
      if (event.shiftKey) {
        degree1 -= 10;
      } else {
        degree1 += 10;
      }
      break;
    case 69: //'e'
      if (event.shiftKey) {
        degree0 -= 10;
      } else {
        degree0 += 10;
      }
      break;
    case 84: //'t'
      if (event.shiftKey) {
        degree2 -= 10;
      } else {
        degree2 += 10;
      }
      break;
  }
  drawScene();
}

// This is the entry point from the html
function webGLStart() {
  canvas = document.getElementById("HierarchicalTransformation");
  document.addEventListener("mousedown", onMouseDown, false);
  document.addEventListener("keydown", onKeyPress, false);

  initGL(canvas);
  shaderProgram = initShaders();

  //get locations of attributes declared in the vertex shader
  aPositionLocation = gl.getAttribLocation(shaderProgram, "aPosition");
  uMMatrixLocation = gl.getUniformLocation(shaderProgram, "uMMatrix");
  uColorLocation = gl.getUniformLocation(shaderProgram, "uColor");

  //enable the attribute arrays
  gl.enableVertexAttribArray(aPositionLocation);

  //initialize buffers for the square
  initSquareBuffer();

  //now draw the scene
  drawScene();
}
