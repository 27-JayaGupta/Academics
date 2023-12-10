////////////////////////////////////////////////////////////////////////////////
//  A simple WebGL program to draw on mutliple viewports by dividing the canvas.
//

var gl;

const vertexShaderCode = `#version 300 es
in vec2 aPosition;
in vec3 aColor;
out vec3 fColor;

void main() {
  fColor = aColor;
  gl_Position = vec4(aPosition,0.0,1.0);
}`;

const fragShaderCode = `#version 300 es
precision mediump float;
out vec4 fragColor;
in vec3 fColor;

void main() {
  fragColor = vec4(fColor, 1.0);
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

////////////////////////////////////////////////////////////////////////
// The main drawing routine, but does nothing except clearing the canvas
//
function drawScene() {
  // You need to enable scissor_test to be able to use multiple viewports
  gl.enable(gl.SCISSOR_TEST);

  //get locations of attributes declared in the vertex shader
  const aPositionLocation = gl.getAttribLocation(shaderProgram, "aPosition");
  const aColorLocation = gl.getAttribLocation(shaderProgram, "aColor");

  // buffer for the three points
  const bufData = new Float32Array([
    0.0, 0.5, 1.0, 0.0, 0.0, -0.5, -0.5, 0.0, 1.0, 0.0, 0.5, -0.5, 0.0, 0.0,
    1.0,
  ]);
  const buf = gl.createBuffer();
  // decide where to copy the data in GPU memory by binding to it
  gl.bindBuffer(gl.ARRAY_BUFFER, buf);
  // copy data from CPU buffer to GPU memory
  gl.bufferData(gl.ARRAY_BUFFER, bufData, gl.STATIC_DRAW);

  gl.vertexAttribPointer(aPositionLocation, 2, gl.FLOAT, false, 5 * 4, 0);
  gl.vertexAttribPointer(aColorLocation, 3, gl.FLOAT, false, 5 * 4, 2 * 4);

  //enable the attribute arrays
  gl.enableVertexAttribArray(aPositionLocation);
  gl.enableVertexAttribArray(aColorLocation);

  // Now define 4 different viewport areas for drawing

  ////////////////////////////////////////
  // Lower left viewport area
  gl.viewport(0, 0, 300, 300);
  gl.scissor(0, 0, 300, 300);
  gl.clearColor(0.9, 0.9, 0.9, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
  gl.drawArrays(gl.TRIANGLES, 0, 3);

  ////////////////////////////////////////
  // Lower right viewport area
  gl.viewport(300, 0, 300, 300);
  gl.scissor(300, 0, 300, 300);
  gl.clearColor(0.75, 0.75, 0.75, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
  gl.drawArrays(gl.TRIANGLES, 0, 3);

  ////////////////////////////////////////
  // upper right viewport area
  gl.viewport(300, 300, 300, 300);
  gl.scissor(300, 300, 300, 300);
  gl.clearColor(0.4, 0.4, 0.4, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
  gl.drawArrays(gl.TRIANGLES, 0, 3);

  ////////////////////////////////////////
  // upper left viewport area
  gl.viewport(0, 300, 300, 300);
  gl.scissor(0, 300, 300, 300);
  gl.clearColor(0.2, 0.2, 0.2, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
  gl.drawArrays(gl.TRIANGLES, 0, 3);
}

// This is the entry point from the html
function webGLStart() {
  var canvas = document.getElementById("viewport");
  initGL(canvas);
  shaderProgram = initShaders();
  drawScene();
}
