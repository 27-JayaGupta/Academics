/*
TODO: Check Flat shading model, gives very rubbish. Mail prof about the sphere Buffer code he provided and ask if there is any problem with it. -> Done
On Rotating the object, the light also gets rotated.
For Gouraud Shading, place the object correctly. As of now there is space between objects which does not looks good on rotation.
*/

var buf;
var indexBuf;
var cubeNormalBuf;
var spBuf;
var spIndexBuf;
var spNormalBuf;

var spVerts = [];
var spIndicies = [];
var spNormals = [];

var matrixStack = [];

// Shader programs for different types of shading
var FlatShaderProgram;
var GouraudShaderProgram;
var PhongShaderProgram;

var shaderInfo = {
  flatShader: {},
  gouraudShader: {},
  phongShader: {}
};

var degree0_flat = 0.0;
var degree1_flat = 0.0;
var degree0_gouraud = 0.0;
var degree1_gouraud = 0.0;
var degree0_phong = 0.0;
var degree1_phong = 0.0;

// specify camera/eye coordinate system parameters
var eyePos = [0.0, 0.0, 4.0];
var COI = [0.0, 0.0, 0.0];
var viewUp = [0.0, 1.0, 0.0];

var lightPos = [100.0, 100.0, 100.0];

// Vertex shader code for Flat shading
const vertexShaderCode_FlatShading = `#version 300 es
in vec3 aPosition;

uniform mat4 uMMatrix;
uniform mat4 uPMatrix;
uniform mat4 uVMatrix;
uniform vec4 objColor;

out vec3 v_posInEyeSpace;
out vec4 v_color;
flat out mat4 v_viewMatrix;

void main() {
  mat4 projectionModelView;
	projectionModelView=uPMatrix*uVMatrix*uMMatrix;

  // Position in eye space and send to fragment shader
  v_posInEyeSpace = vec3(uVMatrix * uMMatrix * vec4(aPosition, 1.0));

  v_color = objColor;
  v_viewMatrix = uVMatrix;

  // Position of point in clip space
  gl_Position = projectionModelView*vec4(aPosition,1.0);
  gl_PointSize=5.0;
}`;

// Fragment shader code for Flat Shading
const fragShaderCode_FlatShading = `#version 300 es

precision mediump float;

in vec3 v_posInEyeSpace;
in vec4 v_color;
flat in mat4 v_viewMatrix;

uniform vec3 lightPos;

out vec4 fragColor;

void main() {
  float intensity = 1.0 ;
  float ambient_percentage = 0.4;
  float shininess = 30.0;
  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;
  vec3 v_lightPos;
  vec3 to_light;
  vec3 vertex_normal;
  float cos_theta;
  float cos_phi;
  vec3 reflection;
  vec3 to_camera;
  vec3 light_color = vec3(1.0, 1.0, 1.0);

  // Calculate ambient color
  ambient_color = ambient_percentage*vec3(v_color);

  // Calculate light position in view space
  v_lightPos = vec3(v_viewMatrix * vec4(lightPos, 1.0));

  // Calculate a vector from the fragment location to the light source
  to_light = v_lightPos - v_posInEyeSpace;
  to_light = normalize( to_light );

  // Change it to face normal, calcualed using dFdx and dFdy
  vertex_normal = normalize(cross(dFdx(v_posInEyeSpace), dFdy(v_posInEyeSpace)));
  
  cos_theta = dot(vertex_normal, to_light);
  cos_theta = clamp(cos_theta, 0.0, 1.0);

  diffuse_color = vec3(v_color) * cos_theta;

  reflection = 2.0 * dot(vertex_normal, to_light) *  vertex_normal - to_light;
  reflection = normalize(reflection);

  to_camera = -1.0 * v_posInEyeSpace;
  to_camera = normalize(to_camera);

  cos_phi = dot(reflection, to_camera);
  cos_phi = clamp(cos_phi, 0.0, 1.0);
  cos_phi = pow(cos_phi, shininess);
  
  specular_color = vec3(v_color) * cos_phi;

  // Specular color is from the reflection of the light from the surface. If light is behind the surface, no color will be seen. Only black object
  if (cos_theta < 0.0) {
    diffuse_color = vec3(0.0, 0.0, 0.0);
    specular_color = vec3(0.0, 0.0, 0.0);
  } 

  fragColor = vec4(intensity * (ambient_color + diffuse_color + 0.8*specular_color), 1.0);
}`;


// ################################################################

// Vertex shader code for Gouraud Shading
const vertexShaderCode_GouraudShading = `#version 300 es
in vec3 aPosition;
in vec3 normalPosition;

uniform mat4 uMMatrix;
uniform mat4 uPMatrix;
uniform mat4 uVMatrix;
uniform mat4 uNormalMatrix;
uniform vec4 objColor;
uniform vec3 lightPos;

out vec4 v_color;

void main() {
  mat4 projectionModelView;
  vec3 v_posInEyeSpace;
  float intensity = 1.0 ;
  float ambient_percentage = 0.4;
  float shininess = 5.0;
  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;
  vec3 v_lightPos;
  vec3 to_light;
  vec3 vertex_normal;
  float cos_theta;
  float cos_phi;
  vec3 reflection;
  vec3 to_camera;
  vec3 light_color = vec3(1.0, 1.0, 1.0);

	projectionModelView=uPMatrix*uVMatrix*uMMatrix;

  // Position of point in clip space
  gl_Position = projectionModelView*vec4(aPosition,1.0);
  gl_PointSize=5.0;

  // Position in eye space
  v_posInEyeSpace = vec3(uVMatrix * uMMatrix * vec4(aPosition, 1.0));

  // Compute normal in eye space
  vertex_normal = vec3(uNormalMatrix * vec4(normalPosition, 0.0));
  vertex_normal = normalize(vertex_normal);

  // Calculate ambient color
  ambient_color = ambient_percentage*vec3(objColor);

  // Calculate light position in view space
  v_lightPos = vec3(uVMatrix * vec4(lightPos, 1.0));

  // Calculate a vector from the fragment location to the light source
  to_light = v_lightPos - v_posInEyeSpace;
  to_light = normalize( to_light );
  
  cos_theta = dot(vertex_normal, to_light);
  cos_theta = clamp(cos_theta, 0.0, 1.0);

  diffuse_color = vec3(objColor) * cos_theta;

  reflection = 2.0 * dot(vertex_normal, to_light) *  vertex_normal - to_light;
  reflection = normalize(reflection);

  to_camera = -1.0 * v_posInEyeSpace;
  to_camera = normalize(to_camera);

  cos_phi = dot(reflection, to_camera);
  cos_phi = clamp(cos_phi, 0.0, 1.0);
  cos_phi = pow(cos_phi, shininess);
  
  specular_color = vec3(objColor) * cos_phi;

  // Specular color is from the reflection of the light from the surface. If light is behind the surface, no color will be seen. Only black object
  if (cos_theta <= 0.0) {
    diffuse_color = vec3(0.0, 0.0, 0.0);
    specular_color = vec3(0.0, 0.0, 0.0);
  } 

  v_color = vec4(intensity * (ambient_color + diffuse_color + 0.8*specular_color), 1.0);
}`;

// Fragment shader code for Gouraud Shading
const fragShaderCode_GouraudShading = `#version 300 es
precision mediump float;

in vec4 v_color;

out vec4 fragColor;

void main() {
  fragColor = v_color;
}`;

// ################################################################

// Vertex shader code for Phong Shading
const vertexShaderCode_PhongShading = `#version 300 es
in vec3 aPosition;
in vec3 normalPosition;

uniform mat4 uMMatrix;
uniform mat4 uPMatrix;
uniform mat4 uVMatrix;
uniform mat4 uNormalMatrix;
uniform vec4 objColor;

out vec3 v_posInEyeSpace;
out vec3 v_normal;
out vec4 v_color;
flat out mat4 v_viewMatrix;

void main() {
  mat4 projectionModelView;
	projectionModelView=uPMatrix*uVMatrix*uMMatrix;

  // Position in eye space and send to fragment shader
  v_posInEyeSpace = vec3(uVMatrix * uMMatrix * vec4(aPosition, 1.0));

  // Compute normal in eye space and send to fragment shader. We will use the model view matrix itself, as there are no non uniform scaling taking place
  v_normal = vec3(uNormalMatrix * vec4(normalPosition, 0.0));

  v_color = objColor;
  v_viewMatrix = uVMatrix;

  // Position of point in clip space
  gl_Position = projectionModelView*vec4(aPosition,1.0);
  gl_PointSize=5.0;
}`;

// Fragment shader code for Phong Shading
const fragShaderCode_PhongShading = `#version 300 es

precision mediump float;

in vec3 v_posInEyeSpace;
in vec3 v_normal;
in vec4 v_color;
flat in mat4 v_viewMatrix;

uniform vec3 lightPos;

out vec4 fragColor;

void main() {
  float intensity = 1.0 ;
  float ambient_percentage = 0.35;
  float shininess = 30.0;
  vec3 ambient_color;
  vec3 diffuse_color;
  vec3 specular_color;
  vec3 v_lightPos;
  vec3 to_light;
  vec3 vertex_normal;
  float cos_theta;
  float cos_phi;
  vec3 reflection;
  vec3 to_camera;
  vec3 light_color = vec3(1.0, 1.0, 1.0);

  // Calculate ambient color
  ambient_color = ambient_percentage*vec3(v_color);

  // Calculate light position in view space
  v_lightPos = vec3(v_viewMatrix * vec4(lightPos, 1.0));

  // Calculate a vector from the fragment location to the light source
  to_light = v_lightPos - v_posInEyeSpace;
  to_light = normalize( to_light );

  // Change it to face normal, calcualed using dFdx and dFdy
  vertex_normal = normalize(v_normal);
  
  cos_theta = dot(vertex_normal, to_light);
  cos_theta = clamp(cos_theta, 0.0, 1.0);

  diffuse_color = vec3(v_color) * cos_theta;

  reflection = 2.0 * dot(vertex_normal, to_light) *  vertex_normal - to_light;
  reflection = normalize(reflection);

  to_camera = -1.0 * v_posInEyeSpace;
  to_camera = normalize(to_camera);

  cos_phi = dot(reflection, to_camera);
  cos_phi = clamp(cos_phi, 0.0, 1.0);
  cos_phi = pow(cos_phi, shininess);
  
  specular_color = vec3(light_color) * cos_phi;

  // Specular color is from the reflection of the light from the surface. If light is behind the surface, no color will be seen. Only black object
  if (cos_theta <= 0.0) {
    diffuse_color = vec3(0.0, 0.0, 0.0);
    specular_color = vec3(0.0, 0.0, 0.0);
  } 

  fragColor = vec4(intensity * (ambient_color + diffuse_color + 0.8*specular_color), 1.0);
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

function initShaders(vertexShaderCode, fragShaderCode) {
  var shaderProgram = gl.createProgram();

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

  return shaderProgram;
}

function getColor(r, g, b) {
  color = [];
  color.push(r/255.0);
  color.push(g/255.0);
  color.push(b/255.0);
  color.push(1.0);
  
  return color;
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

function initSphere(nslices, nstacks, radius) {
  var theta1, theta2;

  for (i = 0; i < nslices; i++) {
    spVerts.push(0);
    spVerts.push(-radius);
    spVerts.push(0);

    spNormals.push(0);
    spNormals.push(-1.0);
    spNormals.push(0);
  }

  for (j = 1; j < nstacks - 1; j++) {
    theta1 = (j * 2 * Math.PI) / nslices - Math.PI / 2;
    for (i = 0; i < nslices; i++) {
      theta2 = (i * 2 * Math.PI) / nslices;
      spVerts.push(radius * Math.cos(theta1) * Math.cos(theta2));
      spVerts.push(radius * Math.sin(theta1));
      spVerts.push(radius * Math.cos(theta1) * Math.sin(theta2));

      spNormals.push(Math.cos(theta1) * Math.cos(theta2));
      spNormals.push(Math.sin(theta1));
      spNormals.push(Math.cos(theta1) * Math.sin(theta2));
    }
  }

  for (i = 0; i < nslices; i++) {
    spVerts.push(0);
    spVerts.push(radius);
    spVerts.push(0);

    spNormals.push(0);
    spNormals.push(1.0);
    spNormals.push(0);
  }

  // setup the connectivity and indices
  for (j = 0; j < nstacks - 1; j++)
    for (i = 0; i <= nslices; i++) {
      var mi = i % nslices;
      var mi2 = (i + 1) % nslices;
      var idx = (j + 1) * nslices + mi;
      var idx2 = j * nslices + mi;
      var idx3 = j * nslices + mi2;
      var idx4 = (j + 1) * nslices + mi;
      var idx5 = j * nslices + mi2;
      var idx6 = (j + 1) * nslices + mi2;

      spIndicies.push(idx);
      spIndicies.push(idx2);
      spIndicies.push(idx3);
      spIndicies.push(idx4);
      spIndicies.push(idx5);
      spIndicies.push(idx6);
    }
}

function initSphereBuffer() {
  var nslices = 30; // use even number
  var nstacks = nslices / 2 + 1;
  var radius = 1.0;
  initSphere(nslices, nstacks, radius);

  spBuf = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, spBuf);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(spVerts), gl.STATIC_DRAW);
  spBuf.itemSize = 3;
  spBuf.numItems = nslices * nstacks;

  spNormalBuf = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, spNormalBuf);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(spNormals), gl.STATIC_DRAW);
  spNormalBuf.itemSize = 3;
  spNormalBuf.numItems = nslices * nstacks;

  spIndexBuf = gl.createBuffer();
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, spIndexBuf);
  gl.bufferData(
    gl.ELEMENT_ARRAY_BUFFER,
    new Uint32Array(spIndicies),
    gl.STATIC_DRAW
  );
  spIndexBuf.itemsize = 1;
  spIndexBuf.numItems = (nstacks - 1) * 6 * (nslices + 1);
}

function drawSphere(color, info) {
  gl.bindBuffer(gl.ARRAY_BUFFER, spBuf);
  gl.vertexAttribPointer(
    info.aPositionLocation,
    spBuf.itemSize,
    gl.FLOAT,
    false,
    0,
    0
  );

  // will be invoked for phong and gouraud shader only
  if(info.normalInfoRequired){
    gl.bindBuffer(gl.ARRAY_BUFFER, spNormalBuf);
    gl.vertexAttribPointer(
      info.normalPositionLocation,
      spNormalBuf.itemSize,
      gl.FLOAT,
      false,
      0,
      0
    );
  }

  gl.uniform4fv(info.uColorLocation, color);
  gl.uniform3fv(info.lightLocation, lightPos);
  gl.uniformMatrix4fv(info.uMMatrixLocation, false, info.mMatrix);
  gl.uniformMatrix4fv(info.uVMatrixLocation, false, info.vMatrix);
  gl.uniformMatrix4fv(info.uPMatrixLocation, false, info.pMatrix);

  // will be invoked for phong and gouraud shader only
  if(info.normalInfoRequired) {
    gl.uniformMatrix4fv(info.uNormalMatrixLocation, false, info.normalMatrix);
  }

  // draw elementary arrays - triangle indices
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, spIndexBuf);
  gl.drawElements(gl.TRIANGLES, spIndexBuf.numItems, gl.UNSIGNED_INT, 0);
}

// Cube generation function with normals
function initCubeBuffer() {
  var vertices = [
    // Front face
    -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5,
    // Back face
    -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, -0.5,
    // Top face
    -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, -0.5, 0.5, 0.5,
    // Bottom face
    -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, -0.5, 0.5, -0.5, -0.5, 0.5,
    // Right face
    0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5, 0.5, 0.5, 0.5, -0.5, 0.5,
    // Left face
    -0.5, -0.5, -0.5, -0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5,
  ];
  buf = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, buf);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);
  buf.itemSize = 3;
  buf.numItems = vertices.length / 3;

  var normals = [
    // Front face
    0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0,
    // Back face
    0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0,
    // Top face
    0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0,
    // Bottom face
    0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0,
    // Right face
    1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 0.0,
    // Left face
    -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0,
  ];
  cubeNormalBuf = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, cubeNormalBuf);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(normals), gl.STATIC_DRAW);
  cubeNormalBuf.itemSize = 3;
  cubeNormalBuf.numItems = normals.length / 3;


  var indices = [
    0,
    1,
    2,
    0,
    2,
    3, // Front face
    4,
    5,
    6,
    4,
    6,
    7, // Back face
    8,
    9,
    10,
    8,
    10,
    11, // Top face
    12,
    13,
    14,
    12,
    14,
    15, // Bottom face
    16,
    17,
    18,
    16,
    18,
    19, // Right face
    20,
    21,
    22,
    20,
    22,
    23, // Left face
  ];
  indexBuf = gl.createBuffer();
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuf);
  gl.bufferData(
    gl.ELEMENT_ARRAY_BUFFER,
    new Uint16Array(indices),
    gl.STATIC_DRAW
  );
  indexBuf.itemSize = 1;
  indexBuf.numItems = indices.length;
}

function drawCube(color, info) {
  gl.bindBuffer(gl.ARRAY_BUFFER, buf);
  gl.vertexAttribPointer(
    info.aPositionLocation,
    buf.itemSize,
    gl.FLOAT,
    false,
    0,
    0
  );
  
  // will be invoked for phong and gouraud shader only
  if(info.normalInfoRequired){
    gl.bindBuffer(gl.ARRAY_BUFFER, cubeNormalBuf);
    gl.vertexAttribPointer(
      info.normalPositionLocation,
      cubeNormalBuf.itemSize,
      gl.FLOAT,
      false,
      0,
      0
    );
  }

  // draw elementary arrays - triangle indices
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuf);

  gl.uniform4fv(info.uColorLocation, color);
  gl.uniform3fv(info.lightLocation, lightPos);
  gl.uniformMatrix4fv(info.uMMatrixLocation, false, info.mMatrix);
  gl.uniformMatrix4fv(info.uVMatrixLocation, false, info.vMatrix);
  gl.uniformMatrix4fv(info.uPMatrixLocation, false, info.pMatrix);

  if(info.normalInfoRequired) {
    gl.uniformMatrix4fv(info.uNormalMatrixLocation, false, info.normalMatrix);
  }

  gl.drawElements(gl.TRIANGLES, indexBuf.numItems, gl.UNSIGNED_SHORT, 0);
}

function createNormalMatrix(mMatrix_N, viewMatrix_N) {
  // setup normal matrix
  normalMatrix = mat4.create();
  mat4.identity(normalMatrix);

  multiply = mat4.create();
  mat4.identity(multiply);
  multiply = mat4.multiply(mMatrix_N, viewMatrix_N, multiply);

  inverse = mat4.create();
  mat4.identity(inverse);
  inverse = mat4.inverse(multiply, inverse);

  normalMatrix = mat4.transpose(inverse, normalMatrix);

  return normalMatrix;
}

function drawFlatShaderDiagram() {
  // setup shader code
  gl.useProgram(FlatShaderProgram);

  //enable the attribute arrays
  gl.enableVertexAttribArray(shaderInfo.flatShader.aPositionLocation);

  // set up the view matrix, multiply into the modelview matrix
  mat4.identity(shaderInfo.flatShader.vMatrix);
  shaderInfo.flatShader.vMatrix = mat4.lookAt(eyePos, COI, viewUp, shaderInfo.flatShader.vMatrix);
  // shaderInfo.flatShader.vMatrix = mat4.rotate(shaderInfo.flatShader.vMatrix, degToRad(-1*degree0_flat), [0,1,0]);
  // shaderInfo.flatShader.vMatrix = mat4.rotate(shaderInfo.flatShader.vMatrix, degToRad(-1*degree1_flat), [1,0,0]);

  //set up perspective projection matrix
  mat4.identity(shaderInfo.flatShader.pMatrix);
  mat4.perspective(40, 1.0, 0.1, 1000, shaderInfo.flatShader.pMatrix);

  //set up the model matrix
  mMatrix = mat4.create();
  mat4.identity(mMatrix);

  // transformations applied here on model matrix
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [0, -0.5, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(degree0_flat), [0, 1, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(degree1_flat), [1, 0, 0]);
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.rotate(mMatrix, degToRad(30), [0, 1, 0]);
  mMatrix = mat4.scale(mMatrix, [0.5, 1.0, 0.5]);
  shaderInfo.flatShader.mMatrix = mMatrix;
  drawCube(getColor(179,180,121), shaderInfo.flatShader);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix)
  mMatrix = mat4.translate(mMatrix, [0, 0.8, 0]);
  mMatrix = mat4.scale(mMatrix, [0.3, 0.3, 0.3]);
  shaderInfo.flatShader.mMatrix = mMatrix;
  drawSphere(getColor(1,131,196), shaderInfo.flatShader);
  mMatrix = popMatrix(matrixStack);
  mMatrix = popMatrix(matrixStack);
}

function drawGouraudShaderDiagram() {

  // setup shader code
  gl.useProgram(GouraudShaderProgram);

  //enable the attribute arrays
  gl.enableVertexAttribArray(shaderInfo.gouraudShader.aPositionLocation);
  // enable the normal arrays
  gl.enableVertexAttribArray(shaderInfo.gouraudShader.normalPositionLocation);

  // set up the view matrix, multiply into the modelview matrix
  mat4.identity(shaderInfo.gouraudShader.vMatrix);
  shaderInfo.gouraudShader.vMatrix = mat4.lookAt(eyePos, COI, viewUp, shaderInfo.gouraudShader.vMatrix);

  //set up perspective projection matrix
  mat4.identity(shaderInfo.gouraudShader.pMatrix);
  mat4.perspective(40, 1.0, 0.1, 1000, shaderInfo.gouraudShader.pMatrix);

  mat4.identity(shaderInfo.gouraudShader.normalMatrix);

  //set up the model matrix
  mMatrix = mat4.create();
  mat4.identity(mMatrix);

  // transformations applied here on model matrix
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [-0.4, -0.4, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(degree0_gouraud), [0, 1, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(degree1_gouraud), [1, 0, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(-30), [0, 0, 1]);
  mMatrix = mat4.rotate(mMatrix, degToRad(-10), [0, 1, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(10), [1, 0, 0]);
  mMatrix = mat4.scale(mMatrix, [0.9, 0.9, 0.9]);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.scale(mMatrix, [0.6, 0.6, 0.6]);
  shaderInfo.gouraudShader.mMatrix = mMatrix;
  shaderInfo.gouraudShader.normalMatrix = createNormalMatrix(shaderInfo.gouraudShader.mMatrix, shaderInfo.gouraudShader.vMatrix);
  drawCube(getColor(3,168,5), shaderInfo.gouraudShader);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [0.8, -0.2, 0]);
  mMatrix = mat4.scale(mMatrix, [0.5, 0.5, 0.5]);
  shaderInfo.gouraudShader.mMatrix = mMatrix;
  shaderInfo.gouraudShader.normalMatrix = createNormalMatrix(shaderInfo.gouraudShader.mMatrix, shaderInfo.gouraudShader.vMatrix);
  drawSphere(getColor(139,139,139), shaderInfo.gouraudShader);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [0, 0.6, 0]);
  mMatrix = mat4.scale(mMatrix, [0.3, 0.3, 0.3]);
  shaderInfo.gouraudShader.mMatrix = mMatrix;
  shaderInfo.gouraudShader.normalMatrix = createNormalMatrix(shaderInfo.gouraudShader.mMatrix, shaderInfo.gouraudShader.vMatrix);
  drawSphere(getColor(139,139,139), shaderInfo.gouraudShader);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [0.28, 1.0, -0.1]);
  mMatrix = mat4.rotate(mMatrix, degToRad(-30), [0, 0, 1]);
  mMatrix = mat4.rotate(mMatrix, degToRad(-15), [1, 0, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(15), [0, 1, 0]);
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.scale(mMatrix, [0.4, 0.4, 0.4]);
  shaderInfo.gouraudShader.mMatrix = mMatrix;
  shaderInfo.gouraudShader.normalMatrix = createNormalMatrix(shaderInfo.gouraudShader.mMatrix, shaderInfo.gouraudShader.vMatrix);
  drawCube(getColor(3,168,5), shaderInfo.gouraudShader);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [-0.4, 0, 0]);
  mMatrix = mat4.scale(mMatrix, [0.2, 0.2, 0.2]);
  shaderInfo.gouraudShader.mMatrix = mMatrix;
  shaderInfo.gouraudShader.normalMatrix = createNormalMatrix(shaderInfo.gouraudShader.mMatrix, shaderInfo.gouraudShader.vMatrix);
  drawSphere(getColor(139,139,139), shaderInfo.gouraudShader);
  mMatrix = popMatrix(matrixStack);
  mMatrix = popMatrix(matrixStack);
  mMatrix = popMatrix(matrixStack);
}

function drawPhongShaderProgram() {
  // setup shader code
  gl.useProgram(PhongShaderProgram);

  //enable the attribute arrays
  gl.enableVertexAttribArray(shaderInfo.phongShader.aPositionLocation);
  // enable the normal arrays
  gl.enableVertexAttribArray(shaderInfo.phongShader.normalPositionLocation);

  // set up the view matrix, multiply into the modelview matrix
  mat4.identity(shaderInfo.phongShader.vMatrix);
  shaderInfo.phongShader.vMatrix = mat4.lookAt(eyePos, COI, viewUp, shaderInfo.phongShader.vMatrix);

  //set up perspective projection matrix
  mat4.identity(shaderInfo.phongShader.pMatrix);
  mat4.perspective(40, 1.0, 0.1, 1000, shaderInfo.phongShader.pMatrix);

  mat4.identity(shaderInfo.phongShader.normalMatrix);

  //set up the model matrix
  //set up the model matrix
  mMatrix = mat4.create();
  mat4.identity(mMatrix);

  // transformations applied here on model matrix
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [0, -0.8, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(degree0_phong), [0, 1, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(degree1_phong), [1, 0, 0]);
  mMatrix = mat4.scale(mMatrix, [0.5, 0.5, 0.5]);
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.scale(mMatrix, [0.7, 0.7, 0.7]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  shaderInfo.phongShader.normalMatrix = createNormalMatrix(shaderInfo.phongShader.mMatrix, shaderInfo.phongShader.vMatrix);
  drawSphere(getColor(0,171,31), shaderInfo.phongShader);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [0, 0.7, 0]);
  mMatrix = mat4.scale(mMatrix, [4, 0.1, 1]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  shaderInfo.phongShader.normalMatrix = createNormalMatrix(shaderInfo.phongShader.mMatrix, shaderInfo.phongShader.vMatrix);
  drawCube(getColor(140,48,19), shaderInfo.phongShader);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [-1.3, 1.15, 0]);
  mMatrix = mat4.scale(mMatrix, [0.4, 0.4, 0.4]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  shaderInfo.phongShader.normalMatrix = createNormalMatrix(shaderInfo.phongShader.mMatrix, shaderInfo.phongShader.vMatrix);
  drawSphere(getColor(81,82,176), shaderInfo.phongShader);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [1.3, 1.15, 0]);
  mMatrix = mat4.scale(mMatrix, [0.4, 0.4, 0.4]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  shaderInfo.phongShader.normalMatrix = createNormalMatrix(shaderInfo.phongShader.mMatrix, shaderInfo.phongShader.vMatrix);
  drawSphere(getColor(23,68,79), shaderInfo.phongShader);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [-1.3, 1.6, 0]);
  mMatrix = mat4.scale(mMatrix, [1.5, 0.1, 3]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  shaderInfo.phongShader.normalMatrix = createNormalMatrix(shaderInfo.phongShader.mMatrix, shaderInfo.phongShader.vMatrix);
  drawCube(getColor(145,145,1), shaderInfo.phongShader);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [1.3, 1.6, 0]);
  mMatrix = mat4.scale(mMatrix, [1.5, 0.1, 3]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  shaderInfo.phongShader.normalMatrix = createNormalMatrix(shaderInfo.phongShader.mMatrix, shaderInfo.phongShader.vMatrix);
  drawCube(getColor(40,146,114), shaderInfo.phongShader);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [-1.3, 2.05, 0]);
  mMatrix = mat4.scale(mMatrix, [0.4, 0.4, 0.4]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  shaderInfo.phongShader.normalMatrix = createNormalMatrix(shaderInfo.phongShader.mMatrix, shaderInfo.phongShader.vMatrix);
  drawSphere(getColor(178,1,179), shaderInfo.phongShader);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [1.3, 2.05, 0]);
  mMatrix = mat4.scale(mMatrix, [0.4, 0.4, 0.4]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  shaderInfo.phongShader.normalMatrix = createNormalMatrix(shaderInfo.phongShader.mMatrix, shaderInfo.phongShader.vMatrix);
  drawSphere(getColor(178,125,45), shaderInfo.phongShader);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [0, 2.45, 0]);
  mMatrix = mat4.scale(mMatrix, [4, 0.1, 1]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  shaderInfo.phongShader.normalMatrix = createNormalMatrix(shaderInfo.phongShader.mMatrix, shaderInfo.phongShader.vMatrix);
  drawCube(getColor(140,48,19), shaderInfo.phongShader);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [0, 3.2, 0]);
  mMatrix = mat4.scale(mMatrix, [0.7, 0.7, 0.7]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  shaderInfo.phongShader.normalMatrix = createNormalMatrix(shaderInfo.phongShader.mMatrix, shaderInfo.phongShader.vMatrix);
  drawSphere(getColor(126,125,162), shaderInfo.phongShader);
  mMatrix = popMatrix(matrixStack);
  mMatrix = popMatrix(matrixStack);
}

function drawScene() {

  // You need to enable scissor_test to be able to use multiple viewports
  gl.enable(gl.SCISSOR_TEST);

  // Now define 3 different viewport areas for drawing

  ////////////////////////////////////////
  // First Viewport(Leftmost)
  gl.viewport(0, 0, 500, 500);
  gl.scissor(0, 0, 500, 500);
  gl.clearColor(0.8274, 0.8274, 0.933, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

  drawFlatShaderDiagram();

  ////////////////////////////////////////
  // Second Viewport(Middle)
  gl.viewport(500, 0, 500, 500);
  gl.scissor(500, 0, 500, 500);
  gl.clearColor(0.933, 0.8274, 0.8235, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

  drawGouraudShaderDiagram();

  ////////////////////////////////////////
  // Third ViewPort(Rightmost)
  gl.viewport(1000, 0, 500, 500);
  gl.scissor(1000, 0, 500, 500);
  gl.clearColor(0.8274, 0.933, 0.8274, 1.0);
  gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
  
  drawPhongShaderProgram();
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

  // if in first viewport
  if (
    event.layerX <= canvas.width/3 &&
    event.layerX >= 0 &&
    event.layerY <= canvas.height &&
    event.layerY >= 0
  ) {
    var mouseX = event.clientX;
    var diffX1 = mouseX - prevMouseX;
    prevMouseX = mouseX;
    degree0_flat = degree0_flat + diffX1 / 5;

    var mouseY = canvas.height - event.clientY;
    var diffY2 = mouseY - prevMouseY;
    prevMouseY = mouseY;
    degree1_flat = degree1_flat - diffY2 / 5;

    drawScene();
  }
  else if(
    event.layerX <= 2 * canvas.width/3 &&
    event.layerX > canvas.width/3 &&
    event.layerY <= canvas.height &&
    event.layerY >= 0
  ){
    var mouseX = event.clientX;
    var diffX1 = mouseX - prevMouseX;
    prevMouseX = mouseX;
    degree0_gouraud = degree0_gouraud + diffX1 / 5;

    var mouseY = canvas.height - event.clientY;
    var diffY2 = mouseY - prevMouseY;
    prevMouseY = mouseY;
    degree1_gouraud = degree1_gouraud - diffY2 / 5;

    drawScene();
  }
  else if(
    event.layerX <= canvas.width &&
    event.layerX > 2* canvas.width/3 &&
    event.layerY <= canvas.height &&
    event.layerY >= 0
  ) {
    var mouseX = event.clientX;
    var diffX1 = mouseX - prevMouseX;
    prevMouseX = mouseX;
    degree0_phong = degree0_phong + diffX1 / 5;

    var mouseY = canvas.height - event.clientY;
    var diffY2 = mouseY - prevMouseY;
    prevMouseY = mouseY;
    degree1_phong = degree1_phong - diffY2 / 5;

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
  canvas = document.getElementById("assignment2");
  document.addEventListener("mousedown", onMouseDown, false);

  // initialize WebGL
  initGL(canvas);

  // initialize shader code
  FlatShaderProgram = initShaders(vertexShaderCode_FlatShading, fragShaderCode_FlatShading);
  GouraudShaderProgram = initShaders(vertexShaderCode_GouraudShading, fragShaderCode_GouraudShading);
  PhongShaderProgram = initShaders(vertexShaderCode_PhongShading, fragShaderCode_PhongShading);

  shaderInfo.flatShader = {
    normalInfoRequired: false,
    aPositionLocation: gl.getAttribLocation(FlatShaderProgram, "aPosition"),
    uMMatrixLocation: gl.getUniformLocation(FlatShaderProgram, "uMMatrix"),
    uVMatrixLocation: gl.getUniformLocation(FlatShaderProgram, "uVMatrix"),
    uPMatrixLocation: gl.getUniformLocation(FlatShaderProgram, "uPMatrix"),
    uColorLocation: gl.getUniformLocation(FlatShaderProgram, "objColor"),
    lightLocation: gl.getUniformLocation(FlatShaderProgram, "lightPos"),
    vMatrix: mat4.create(),
    pMatrix: mat4.create(),
    mMatrix: mat4.create()
  };

  shaderInfo.gouraudShader = {
    normalInfoRequired: true,
    aPositionLocation: gl.getAttribLocation(GouraudShaderProgram, "aPosition"),
    normalPositionLocation: gl.getAttribLocation(GouraudShaderProgram, "normalPosition"),
    uMMatrixLocation: gl.getUniformLocation(GouraudShaderProgram, "uMMatrix"),
    uVMatrixLocation: gl.getUniformLocation(GouraudShaderProgram, "uVMatrix"),
    uPMatrixLocation: gl.getUniformLocation(GouraudShaderProgram, "uPMatrix"),
    uNormalMatrixLocation: gl.getUniformLocation(GouraudShaderProgram, "uNormalMatrix"),
    uColorLocation: gl.getUniformLocation(GouraudShaderProgram, "objColor"),
    lightLocation: gl.getUniformLocation(GouraudShaderProgram, "lightPos"),
    vMatrix: mat4.create(),
    pMatrix: mat4.create(),
    mMatrix: mat4.create(),
    normalMatrix: mat4.create(),
  };

  shaderInfo.phongShader = {
    normalInfoRequired: true,
    aPositionLocation: gl.getAttribLocation(PhongShaderProgram, "aPosition"),
    normalPositionLocation: gl.getAttribLocation(PhongShaderProgram, "normalPosition"),
    uMMatrixLocation: gl.getUniformLocation(PhongShaderProgram, "uMMatrix"),
    uVMatrixLocation: gl.getUniformLocation(PhongShaderProgram, "uVMatrix"),
    uPMatrixLocation: gl.getUniformLocation(PhongShaderProgram, "uPMatrix"),
    uNormalMatrixLocation: gl.getUniformLocation(PhongShaderProgram, "uNormalMatrix"),
    uColorLocation: gl.getUniformLocation(PhongShaderProgram, "objColor"),
    lightLocation: gl.getUniformLocation(PhongShaderProgram, "lightPos"),
    vMatrix: mat4.create(),
    pMatrix: mat4.create(),
    mMatrix: mat4.create(),
    normalMatrix: mat4.create(),
  };

  //initialize buffers for the square
  initCubeBuffer();
  initSphereBuffer();

  // enable depth test
  gl.enable(gl.DEPTH_TEST);

  var sliderx = document.getElementById("myRange_X");

  // Update the current slider value (each time you drag the slider handle)
  sliderx.oninput = function() {
    lightPos[0] = this.value;
    drawScene();
  }

  var slidery = document.getElementById("myRange_Y");

  // Update the current slider value (each time you drag the slider handle)
  slidery.oninput = function() {
    lightPos[1] = this.value;
    drawScene();
  }

  var sliderz = document.getElementById("myRange_Z");

  // Update the current slider value (each time you drag the slider handle)
  sliderz.oninput = function() {
    lightPos[2] = this.value;
    drawScene();
  }

  var sliderzoom = document.getElementById("zoom");

  // Update the current slider value (each time you drag the slider handle)
  sliderzoom.oninput = function() {
    eyePos[2] = this.value;
    drawScene();
  }

  drawScene();
}