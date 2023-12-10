////////////////////////////////////////////////////////////////////////
// A simple WebGL program to draw tringles using triangle strip.
// Also shows use of button and how to use that in JS code to change
// background color of the canvas.
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
  gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

  //get locations of attributes declared in the vertex shader
  const aPositionLocation = gl.getAttribLocation(shaderProgram, "aPosition");
  const aColorLocation = gl.getAttribLocation(shaderProgram, "aColor");

  //enable the attribute arrays
  gl.enableVertexAttribArray(aPositionLocation);
  gl.enableVertexAttribArray(aColorLocation);

  // buffer for the four points
  const bufData = new Float32Array([
    0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, -0.5,
  ]);
  const buf = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, buf);
  gl.bufferData(gl.ARRAY_BUFFER, bufData, gl.STATIC_DRAW);
  gl.vertexAttribPointer(aPositionLocation, 2, gl.FLOAT, false, 2 * 4, 0);

  // buffer for the point colors
  const colors = new Float32Array([
    1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0,
  ]);
  const colorBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, colorBuffer);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);
  gl.vertexAttribPointer(aColorLocation, 3, gl.FLOAT, false, 3 * 4, 0);

  // A single draw call is sufficient, see the last parameter=4.
  // It says how many points are being drawn.
  // drawing two triangles with 4 vertices using TRIANGLE_STRIP
  // two points are shared between two triangles so we can save some GPU memory
  gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
}

// This is the entry point from the html
function webGLStart() {
  var canvas = document.getElementById("triangleStripRender");
  initGL(canvas);
  shaderProgram = initShaders();
  gl.clearColor(0.7, 0.7, 0.7, 1.0); // this call is moved out of drawscene() to make the button callback effect .. think about why?
  drawScene();
}

// this function gets called when the button is pressed.
// it changes the background color of the canvas by clearing it with
// some color passed as a parameter from the HTML interface
function changeBGColor(red, green, blue) {
  gl.clearColor(red, green, blue, 1.0);
  drawScene();
}
