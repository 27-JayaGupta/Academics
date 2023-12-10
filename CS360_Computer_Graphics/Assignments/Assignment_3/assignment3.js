/*
TODO: Check Flat shading model, gives very rubbish. Mail prof about the sphere Buffer code he provided and ask if there is any problem with it. -> Done
On Rotating the object, the light also gets rotated.
For Gouraud Shading, place the object correctly. As of now there is space between objects which does not looks good on rotation.
*/
var animation;
var buf;
var indexBuf;
var cubeNormalBuf;
var cubeTextureBuf;
var spBuf;
var spIndexBuf;
var spNormalBuf;
var spTexBuf;
var objVertexPositionBuffer;
var objVertexNormalBuffer;
var objVertexTextureBuffer;
var objVertexIndexBuffer;

var sqVertexPositionBuffer;
var sqVertexTextureBuffer;
var sqVertexNormalBuffer;
var sqVertexIndexBuffer;

// set up these 2 variables before calling any draw function
var lookup2DTex=false;
var refract = false;
var eta = 0.82;

var rcubeTex;
var woodTex;
var cubeMapTex;

var rcubeFileLocation = "texture_and_other_files/rcube.png";
var woodTexPath = "texture_and_other_files/wood_texture.jpg";
var cubeMapPath = "texture_and_other_files/Nvidia_cubemap/";
var input_JSON = "texture_and_other_files/teapot.json";
var posx_file, posy_file, posz_file, negx_file, negy_file, negz_file;
var posx, posy, posz, negx, negy, negz;

var spVerts = [];
var spIndicies = [];
var spNormals = [];
var spTexCoords = [];

var matrixStack = [];
var mMatrix;

var degree0 = 0.0, degree1 = 0.0;
// Shader programs for different types of shading
var PhongShaderProgram;

var shaderInfo = {
  phongShader: {}
};


var rot = 0;
var radius = 20.0;

// specify camera/eye coordinate system parameters
var eyePos = [0.0, 4.0, radius];
var COI = [0.0, 0.0, 0.0];
var viewUp = [0.0, 1.0, 0.0];

var lightPos = [0, 100, 100];

// ################################################################

// Vertex shader code for Phong Shading
const vertexShaderCode_PhongShading = `#version 300 es
in vec3 aPosition;
in vec3 normalPosition;
in vec2 textureCoords;

uniform mat4 uMMatrix;
uniform mat4 uPMatrix;
uniform mat4 uVMatrix;
uniform mat4 uNormalMatrix;
uniform mat4 uwNormalMatrix;
uniform vec4 objColor;

out vec3 v_posInEyeSpace;
out vec3 v_normal;
out vec4 v_color;
out vec2 fragTexCoord;

// for reflection which is done in world space
out vec3 v_worldPosition;
out vec3 v_worldNormal;
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

  fragTexCoord = textureCoords;

  // v_worldPosition = vec3(uMMatrix * vec4(aPosition, 1.0));
  // v_worldNormal = vec3(uwNormalMatrix * vec4(normalPosition, 0.0));

  v_worldPosition = mat3(uMMatrix) * aPosition;
  v_worldNormal = mat3(uwNormalMatrix) * normalPosition;

  // Position of point in clip space
  gl_Position = projectionModelView*vec4(aPosition,1.0);
  gl_PointSize=5.0;
}`;

// Fragment shader code for Phong Shading
const fragShaderCode_PhongShading = `#version 300 es

precision highp float;

in vec3 v_posInEyeSpace;
in vec3 v_normal;
in vec4 v_color;
in vec2 fragTexCoord;

// for reflection which is done in world space
in vec3 v_worldPosition;
in vec3 v_worldNormal;

flat in mat4 v_viewMatrix;

uniform vec3 lightPos;
uniform vec3 eyePosWorld;
uniform samplerCube cubeMap;
uniform sampler2D uTexture;
uniform float reflectivity;
uniform float eta;
uniform bool isRefract;
uniform bool lookup2DTex;

out vec4 fragColor;

void main() {
  float intensity = 1.0 ;
  float ambient_percentage = 0.35;
  float shininess = 10.0;
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
  vec4 obj_color;
  vec4 ref_color;

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

  if(lookup2DTex) {
    obj_color = texture(uTexture, fragTexCoord);
  }
  else {
    obj_color = vec4(intensity * (ambient_color + diffuse_color + specular_color), 1.0);
  }
  
  // calculate reflect color
  vec3 worldNormal = normalize(v_worldNormal);
  vec3 eyeToSurfacePos = normalize(v_worldPosition - eyePosWorld);
  if(isRefract) {
    vec3 refract_vec = normalize(refract(eyeToSurfacePos, worldNormal, eta));
    ref_color = texture(cubeMap, refract_vec);
  }
  else {
    vec3 reflect_vec = normalize(reflect(eyeToSurfacePos, worldNormal));
    ref_color = texture(cubeMap, reflect_vec);
  }

  fragColor = vec4(reflectivity * vec3(ref_color) + (1.0-reflectivity) * vec3(obj_color), 1.0);
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

function setParams(lookUp2D, ref) {
  lookup2DTex = lookUp2D;
  refract = ref;
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

function createWorldNormalMatrix(mMatrix_N) {
  // setup normal matrix
  normalMatrix = mat4.create();
  mat4.identity(normalMatrix);

  inverse = mat4.create();
  mat4.identity(inverse);
  inverse = mat4.inverse(mMatrix_N, inverse);

  normalMatrix = mat4.transpose(inverse, normalMatrix);

  return normalMatrix;
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

function initTextures(textureFile) {
  var tex = gl.createTexture();
  tex.image = new Image();
  tex.image.src = textureFile;
  tex.image.onload = function () {
    handleTextureLoaded(tex);
  };
  return tex;
}

function handleTextureLoaded(texture) {
  gl.bindTexture(gl.TEXTURE_2D, texture);
  gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1); // use it to flip Y if needed
  gl.texImage2D(
    gl.TEXTURE_2D, // 2D texture
    0, // mipmap level
    gl.RGB, // internal format
    gl.RGB, // format
    gl.UNSIGNED_BYTE, // type of data
    texture.image // array or <img>
  );

  gl.generateMipmap(gl.TEXTURE_2D);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
  gl.texParameteri(
    gl.TEXTURE_2D,
    gl.TEXTURE_MIN_FILTER,
    gl.LINEAR_MIPMAP_LINEAR
  );

  drawScene();
}

function initCubeMapPath() {
  posx_file = cubeMapPath.concat("posx.jpg");
  posy_file = cubeMapPath.concat("posy.jpg");
  posz_file = cubeMapPath.concat("posz.jpg");
  negx_file = cubeMapPath.concat("negx.jpg");
  negy_file = cubeMapPath.concat("negy.jpg");
  negz_file = cubeMapPath.concat("negz.jpg");

  posx = initTextures(posx_file);
  posy = initTextures(posy_file);
  posz = initTextures(posz_file);
  negx = initTextures(negx_file);
  negy = initTextures(negy_file);
  negz = initTextures(negz_file);
}

function initCubeMap() {
  const faceInfos = [
    {
      target: gl.TEXTURE_CUBE_MAP_POSITIVE_X,
      url: posx_file,
    },
    {
      target: gl.TEXTURE_CUBE_MAP_NEGATIVE_X,
      url: negx_file,
    },
    {
      target: gl.TEXTURE_CUBE_MAP_POSITIVE_Y,
      url: posy_file,
    },
    {
      target: gl.TEXTURE_CUBE_MAP_NEGATIVE_Y,
      url: negy_file,
    },
    {
      target: gl.TEXTURE_CUBE_MAP_POSITIVE_Z,
      url: posz_file,
    },
    {
      target: gl.TEXTURE_CUBE_MAP_NEGATIVE_Z,
      url: negz_file,
    }
  ];

  cubeMapTex = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_CUBE_MAP, cubeMapTex);

  faceInfos.forEach((faceInfo) => {
    const {target, url} = faceInfo;

    gl.texImage2D(
      target,
      0,
      gl.RGBA,
      512,
      512,
      0,
      gl.RGBA,
      gl.UNSIGNED_BYTE,
      null
    );

    const image = new Image();
    image.src = url;
    image.addEventListener("load", function() {
      gl.bindTexture(gl.TEXTURE_CUBE_MAP, cubeMapTex);
      gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 0); // add this line and see the outcome 
      gl.texImage2D(
        target,
        0,
        gl.RGBA,
        gl.RGBA,
        gl.UNSIGNED_BYTE,
        image
      );
      gl.generateMipmap(gl.TEXTURE_CUBE_MAP);
      drawScene();
    });
  });

  gl.generateMipmap(gl.TEXTURE_CUBE_MAP);
  gl.texParameteri(
    gl.TEXTURE_CUBE_MAP,
    gl.TEXTURE_MIN_FILTER,
    gl.LINEAR_MIPMAP_LINEAR
  );
  gl.texParameteri(
    gl.TEXTURE_CUBE_MAP,
    gl.TEXTURE_MAG_FILTER,
    gl.LINEAR
  );
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
  
  objVertexNormalBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, objVertexNormalBuffer);
  gl.bufferData(
    gl.ARRAY_BUFFER,
    new Float32Array(objData.vertexNormals),
    gl.STATIC_DRAW
  );
  objVertexNormalBuffer.itemSize = 3;
  objVertexNormalBuffer.numItems = objData.vertexNormals.length/3;

  objVertexTextureBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, objVertexTextureBuffer);
  gl.bufferData(
    gl.ARRAY_BUFFER,
    new Float32Array(objData.vertexTextureCoords),
    gl.STATIC_DRAW
  );
  objVertexTextureBuffer.itemSize = 2;
  objVertexTextureBuffer.numItems = objData.vertexNormals.length/2;

  objVertexIndexBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, objVertexIndexBuffer);
  gl.bufferData(
    gl.ELEMENT_ARRAY_BUFFER,
    new Uint32Array(objData.indices),
    gl.STATIC_DRAW
  );
  objVertexIndexBuffer.itemSize = 1;
  objVertexIndexBuffer.numItems = objData.indices.length;
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

function drawObject(color, info, textureUnit2D, textureUnit3D, reflectivity) {

  gl.bindBuffer(gl.ARRAY_BUFFER, objVertexPositionBuffer);
  gl.vertexAttribPointer(
    info.aPositionLocation,
    objVertexPositionBuffer.itemSize,
    gl.FLOAT,
    false,
    0,
    0
  );

  gl.bindBuffer(gl.ARRAY_BUFFER, objVertexTextureBuffer);
  gl.vertexAttribPointer(
    info.aTextureLocation,
    objVertexTextureBuffer.itemSize,
    gl.FLOAT,
    false,
    0,
    0
  );

  // will be invoked for phong and gouraud shader only
  if(info.normalInfoRequired){
    gl.bindBuffer(gl.ARRAY_BUFFER, objVertexNormalBuffer);
    gl.vertexAttribPointer(
      info.normalPositionLocation,
      objVertexNormalBuffer.itemSize,
      gl.FLOAT,
      false,
      0,
      0
    );
  }

  gl.uniform1i(info.aTextureUnit, textureUnit2D);
  gl.uniform1i(info.cubeMapLoc, textureUnit3D);
  
  gl.uniform1f(info.aReflectivity, reflectivity);
  gl.uniform1f(info.etaLoc, eta);
  gl.uniform1i(info.lookup2DTexLoc, lookup2DTex);
  gl.uniform1i(info.refractLoc, refract);
  gl.uniform3fv(info.eyePosWorldLoc, eyePos);
  gl.uniform4fv(info.uColorLocation, color);
  gl.uniform3fv(info.lightLocation, lightPos);
  gl.uniformMatrix4fv(info.uMMatrixLocation, false, info.mMatrix);
  gl.uniformMatrix4fv(info.uVMatrixLocation, false, info.vMatrix);
  gl.uniformMatrix4fv(info.uPMatrixLocation, false, info.pMatrix);

  // will be invoked for phong and gouraud shader only
  if(info.normalInfoRequired) {
    gl.uniformMatrix4fv(info.uNormalMatrixLocation, false, createNormalMatrix(info.mMatrix, info.vMatrix));
    gl.uniformMatrix4fv(info.uwnMatrixLocation, false, createWorldNormalMatrix(info.mMatrix));
  }

  // draw elementary arrays - triangle indices
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, objVertexIndexBuffer);
  gl.drawElements(gl.TRIANGLES, objVertexIndexBuffer.numItems, gl.UNSIGNED_INT, 0);
}

// New sphere initialization function
function initSphere(nslices, nstacks, radius) {
  for (var i = 0; i <= nslices; i++) {
    var angle = (i * Math.PI) / nslices;
    var comp1 = Math.sin(angle);
    var comp2 = Math.cos(angle);

    for (var j = 0; j <= nstacks; j++) {
      var phi = (j * 2 * Math.PI) / nstacks;
      var comp3 = Math.sin(phi);
      var comp4 = Math.cos(phi);

      var xcood = comp4 * comp1;
      var ycoord = comp2;
      var zcoord = comp3 * comp1;
      var utex = 1 - j / nstacks;
      var vtex = 1 - i / nslices;

      spVerts.push(radius * xcood, radius * ycoord, radius * zcoord);
      spNormals.push(xcood, ycoord, zcoord);
      spTexCoords.push(utex, vtex);
    }
  }

  // now compute the indices here
  for (var i = 0; i < nslices; i++) {
    for (var j = 0; j < nstacks; j++) {
      var id1 = i * (nstacks + 1) + j;
      var id2 = id1 + nstacks + 1;

      spIndicies.push(id1, id2, id1 + 1);
      spIndicies.push(id2, id2 + 1, id1 + 1);
    }
  }
}

function initSphereBuffer() {
  var nslices = 50;
  var nstacks = 50;
  var radius = 1.0;

  initSphere(nslices, nstacks, radius);

  // buffer for vertices
  spBuf = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, spBuf);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(spVerts), gl.STATIC_DRAW);
  spBuf.itemSize = 3;
  spBuf.numItems = spVerts.length / 3;

  // buffer for indices
  spIndexBuf = gl.createBuffer();
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, spIndexBuf);
  gl.bufferData(
    gl.ELEMENT_ARRAY_BUFFER,
    new Uint32Array(spIndicies),
    gl.STATIC_DRAW
  );
  spIndexBuf.itemsize = 1;
  spIndexBuf.numItems = spIndicies.length;

  // buffer for normals
  spNormalBuf = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, spNormalBuf);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(spNormals), gl.STATIC_DRAW);
  spNormalBuf.itemSize = 3;
  spNormalBuf.numItems = spNormals.length / 3;

  // buffer for texture coordinates
  spTexBuf = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, spTexBuf);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(spTexCoords), gl.STATIC_DRAW);
  spTexBuf.itemSize = 2;
  spTexBuf.numItems = spTexCoords.length / 2;
}

function drawSphere(color, info, textureUnit2D, textureUnit3D, reflectivity) {
  gl.bindBuffer(gl.ARRAY_BUFFER, spBuf);
  gl.vertexAttribPointer(
    info.aPositionLocation,
    spBuf.itemSize,
    gl.FLOAT,
    false,
    0,
    0
  );

  gl.bindBuffer(gl.ARRAY_BUFFER, spTexBuf);
  gl.vertexAttribPointer(
    info.aTextureLocation,
    spTexBuf.itemSize,
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

  gl.uniform1i(info.aTextureUnit, textureUnit2D);
  gl.uniform1i(info.cubeMapLoc, textureUnit3D);
  
  gl.uniform1f(info.aReflectivity, reflectivity);
  gl.uniform1f(info.etaLoc, eta);
  gl.uniform1i(info.lookup2DTexLoc, lookup2DTex);
  gl.uniform1i(info.refractLoc, refract);
  gl.uniform3fv(info.eyePosWorldLoc, eyePos);
  gl.uniform4fv(info.uColorLocation, color);
  gl.uniform3fv(info.lightLocation, lightPos);
  gl.uniformMatrix4fv(info.uMMatrixLocation, false, info.mMatrix);
  gl.uniformMatrix4fv(info.uVMatrixLocation, false, info.vMatrix);
  gl.uniformMatrix4fv(info.uPMatrixLocation, false, info.pMatrix);

  // will be invoked for phong and gouraud shader only
  if(info.normalInfoRequired) {
    gl.uniformMatrix4fv(info.uNormalMatrixLocation, false, createNormalMatrix(info.mMatrix, info.vMatrix));
    gl.uniformMatrix4fv(info.uwnMatrixLocation, false, createWorldNormalMatrix(info.mMatrix));
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

  var texCoords = [
    0, 0, 0, 1, 1, 1, 1 , 0,
    0, 0, 0, 1, 1, 1, 1 , 0,
    0, 0, 0, 1, 1, 1, 1 , 0,
    0, 0, 0, 1, 1, 1, 1 , 0,
    0, 0, 0, 1, 1, 1, 1 , 0,
    0, 0, 0, 1, 1, 1, 1 , 0,
  ];

  cubeTextureBuf = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, cubeTextureBuf);
  gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(texCoords), gl.STATIC_DRAW);
  cubeTextureBuf.itemSize = 2;
  cubeTextureBuf.numItems = texCoords.length / 2;

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

function drawCube(color, info, textureUnit2D, textureUnit3D, reflectivity) {
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

  // bind texture coordinates
  gl.bindBuffer(gl.ARRAY_BUFFER, cubeTextureBuf);
  gl.vertexAttribPointer(
    info.aTextureLocation,
    cubeTextureBuf.itemSize,
    gl.FLOAT,
    false,
    0,
    0
  );

  // draw elementary arrays - triangle indices
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuf);
  
  gl.uniform1i(info.aTextureUnit, textureUnit2D);
  gl.uniform1i(info.cubeMapLoc, textureUnit3D);

  gl.uniform1f(info.aReflectivity, reflectivity);
  gl.uniform1f(info.etaLoc, eta);
  gl.uniform1i(info.lookup2DTexLoc, lookup2DTex);
  gl.uniform1i(info.refractLoc, refract);
  gl.uniform3fv(info.eyePosWorldLoc, eyePos);
  gl.uniform4fv(info.uColorLocation, color);
  gl.uniform3fv(info.lightLocation, lightPos);
  gl.uniformMatrix4fv(info.uMMatrixLocation, false, info.mMatrix);
  gl.uniformMatrix4fv(info.uVMatrixLocation, false, info.vMatrix);
  gl.uniformMatrix4fv(info.uPMatrixLocation, false, info.pMatrix);

  if(info.normalInfoRequired) {
    gl.uniformMatrix4fv(info.uNormalMatrixLocation, false, createNormalMatrix(info.mMatrix, info.vMatrix));
    gl.uniformMatrix4fv(info.uwnMatrixLocation, false, createWorldNormalMatrix(info.mMatrix));
  }

  gl.drawElements(gl.TRIANGLES, indexBuf.numItems, gl.UNSIGNED_SHORT, 0);
}

function initSquareBuffer() {
  // buffer for point locations
  const sqVertices = new Float32Array([
    0.5, 0.5, 0.0, -0.5, 0.5, 0.0, -0.5, -0.5, 0.0, 0.5, -0.5, 0.0,
  ]);
  sqVertexPositionBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, sqVertexPositionBuffer);
  gl.bufferData(gl.ARRAY_BUFFER, sqVertices, gl.STATIC_DRAW);
  sqVertexPositionBuffer.itemSize = 3;
  sqVertexPositionBuffer.numItems = 4;

  const sqTexture = new Float32Array([
    0, 0, 0, 1, 1, 1, 1, 0
  ]);

  sqVertexTextureBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, sqVertexTextureBuffer);
  gl.bufferData(gl.ARRAY_BUFFER, sqTexture, gl.STATIC_DRAW);
  sqVertexTextureBuffer.itemSize = 2;
  sqVertexTextureBuffer.numItems = 4;

  const sqNormals = new Float32Array([
    0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1,
  ]);

  sqVertexNormalBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, sqVertexNormalBuffer);
  gl.bufferData(gl.ARRAY_BUFFER, sqNormals, gl.STATIC_DRAW);
  sqVertexNormalBuffer.itemSize = 3;
  sqVertexNormalBuffer.numItems = 4;

  // buffer for point indices
  const sqIndices = new Uint16Array([0, 1, 2, 0, 2, 3]);
  sqVertexIndexBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, sqVertexIndexBuffer);
  gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, sqIndices, gl.STATIC_DRAW);
  sqVertexIndexBuffer.itemsize = 1;
  sqVertexIndexBuffer.numItems = 6;
}

function drawSquare(color, info, textureUnit2D, textureUnit3D, reflectivity) {

  // buffer for point locations
  gl.bindBuffer(gl.ARRAY_BUFFER, sqVertexPositionBuffer);
  gl.vertexAttribPointer(
    info.aPositionLocation,
    sqVertexPositionBuffer.itemSize,
    gl.FLOAT,
    false,
    0,
    0
  );

  gl.bindBuffer(gl.ARRAY_BUFFER, sqVertexTextureBuffer);
  gl.vertexAttribPointer(
    info.aTextureLocation,
    sqVertexTextureBuffer.itemSize,
    gl.FLOAT,
    false,
    0,
    0
  );

  // will be invoked for phong and gouraud shader only
  if(info.normalInfoRequired){
    gl.bindBuffer(gl.ARRAY_BUFFER, sqVertexNormalBuffer);
    gl.vertexAttribPointer(
      info.normalPositionLocation,
      sqVertexNormalBuffer.itemSize,
      gl.FLOAT,
      false,
      0,
      0
    );
  }

  // buffer for point indices
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, sqVertexIndexBuffer);

  gl.uniform1i(info.aTextureUnit, textureUnit2D);
  gl.uniform1i(info.cubeMapLoc, textureUnit3D);

  gl.uniform1f(info.aReflectivity, reflectivity);
  gl.uniform1f(info.etaLoc, eta);
  gl.uniform1i(info.lookup2DTexLoc, lookup2DTex);
  gl.uniform1i(info.refractLoc, refract);
  gl.uniform3fv(info.eyePosWorldLoc, eyePos);
  gl.uniform4fv(info.uColorLocation, color);
  gl.uniform3fv(info.lightLocation, lightPos);
  gl.uniformMatrix4fv(info.uMMatrixLocation, false, info.mMatrix);
  gl.uniformMatrix4fv(info.uVMatrixLocation, false, info.vMatrix);
  gl.uniformMatrix4fv(info.uPMatrixLocation, false, info.pMatrix);
  
  
  if(info.normalInfoRequired) {
    gl.uniformMatrix4fv(info.uNormalMatrixLocation, false, createNormalMatrix(info.mMatrix, info.vMatrix));
    gl.uniformMatrix4fv(info.uwnMatrixLocation, false, createWorldNormalMatrix(info.mMatrix));
  }

  gl.drawElements(
    gl.TRIANGLES,
    sqVertexIndexBuffer.numItems,
    gl.UNSIGNED_SHORT,
    0
  );
}

function drawTable() {

  gl.activeTexture(gl.TEXTURE2);
  gl.bindTexture(gl.TEXTURE_2D, woodTex);
  gl.activeTexture(gl.TEXTURE1);
  gl.bindTexture(gl.TEXTURE_CUBE_MAP, cubeMapTex);
  setParams(true, false);

  // transformations applied here on model matrix
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [0, 0.5, 0]);
  mMatrix = mat4.scale(mMatrix, [1.8, 0.05, 1.8]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  drawSphere(getColor(101,200,76), shaderInfo.phongShader, 2, 1, 0.6);
  mMatrix = popMatrix(matrixStack);
  
  // transformations applied here on model matrix
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [-1.0, -0.3, 0.5]);
  mMatrix = mat4.scale(mMatrix, [0.15, 1.5, 0.15]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  drawCube(getColor(63,52,95), shaderInfo.phongShader, 2, 1, 0.6);
  mMatrix = popMatrix(matrixStack);

  // transformations applied here on model matrix
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [1.0, -0.3, 0.5]);
  mMatrix = mat4.scale(mMatrix, [0.15, 1.5, 0.15]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  drawCube(getColor(63,52,95), shaderInfo.phongShader, 2, 1, 0.6);
  mMatrix = popMatrix(matrixStack);

  // transformations applied here on model matrix
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [1.0, -0.3, -0.5]);
  mMatrix = mat4.scale(mMatrix, [0.15, 1.5, 0.15]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  drawCube(getColor(63,52,95), shaderInfo.phongShader, 2, 1, 0.6);
  mMatrix = popMatrix(matrixStack);

  // transformations applied here on model matrix
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [-1.0, -0.3, -0.5]);
  mMatrix = mat4.scale(mMatrix, [0.15, 1.5, 0.15]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  drawCube(getColor(63,52,95), shaderInfo.phongShader, 2, 1, 0.6);
  mMatrix = popMatrix(matrixStack);
}

function drawTeapot() {
  gl.activeTexture(gl.TEXTURE1);
  gl.bindTexture(gl.TEXTURE_CUBE_MAP, cubeMapTex);

  //draw teapot
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.scale(mMatrix, [0.05, 0.05, 0.05]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  setParams(false, false);
  drawObject(getColor(63,52,95), shaderInfo.phongShader, 0, 1, 1)
  mMatrix = popMatrix(matrixStack);
}

function drawSkyBox() {

  // Back side of cube
  pushMatrix(matrixStack, mMatrix);

  gl.activeTexture(gl.TEXTURE3);
  gl.bindTexture(gl.TEXTURE_2D, negz);
   
   // transformations
  mMatrix = mat4.translate(mMatrix, [0, 0, -99.5]);
  mMatrix = mat4.rotate(mMatrix, degToRad(-90), [0, 0, 1]);
  mMatrix = mat4.scale(mMatrix, [200, 200, 200]);
  color = [0.0, 1.0, 1.0, 1.0];
  shaderInfo.phongShader.mMatrix = mMatrix;
  setParams(true, false);
  drawSquare(color, shaderInfo.phongShader, 3, 0, 0);
  mMatrix = popMatrix(matrixStack);

  // Front side of cube
  pushMatrix(matrixStack, mMatrix);

  gl.activeTexture(gl.TEXTURE3);
  gl.bindTexture(gl.TEXTURE_2D, posz);
   
   // transformations
  mMatrix = mat4.translate(mMatrix, [0, 0, 99.5]);
  mMatrix = mat4.rotate(mMatrix, degToRad(-90), [0, 0, 1]);
  mMatrix = mat4.scale(mMatrix, [200, 200, 200]);
  color = [0.0, 1.0, 1.0, 1.0];
  shaderInfo.phongShader.mMatrix = mMatrix;
  setParams(true, false);
  drawSquare(color, shaderInfo.phongShader, 3, 0, 0);
  mMatrix = popMatrix(matrixStack);

  // Left side of cube
  pushMatrix(matrixStack, mMatrix);

  gl.activeTexture(gl.TEXTURE3);
  gl.bindTexture(gl.TEXTURE_2D, negx);
   
   // transformations
  mMatrix = mat4.translate(mMatrix, [-99.5, 0, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(90), [1, 0, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(-90), [0, 1, 0]);
  mMatrix = mat4.scale(mMatrix, [200, 200, 200]);
  color = [0.0, 1.0, 1.0, 1.0];
  shaderInfo.phongShader.mMatrix = mMatrix;
  setParams(true, false);
  drawSquare(color, shaderInfo.phongShader, 3, 0, 0);
  mMatrix = popMatrix(matrixStack);

  // Right side of cube
  pushMatrix(matrixStack, mMatrix);

  gl.activeTexture(gl.TEXTURE3);
  gl.bindTexture(gl.TEXTURE_2D, posx);
   
   // transformations
  mMatrix = mat4.translate(mMatrix, [99.5, 0, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(90), [1, 0, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(-90), [0, 1, 0]);
  mMatrix = mat4.scale(mMatrix, [200, 200, 200]);
  color = [0.0, 1.0, 1.0, 1.0];
  shaderInfo.phongShader.mMatrix = mMatrix;
  setParams(true, false);
  drawSquare(color, shaderInfo.phongShader, 3, 0, 0);
  mMatrix = popMatrix(matrixStack);

  // Top side of cube
  pushMatrix(matrixStack, mMatrix);

  gl.activeTexture(gl.TEXTURE3);
  gl.bindTexture(gl.TEXTURE_2D, posy);
   
   // transformations
  mMatrix = mat4.translate(mMatrix, [0, 99.5, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(-90), [1, 0, 0]);
  mMatrix = mat4.scale(mMatrix, [200, 200, 200]);
  color = [0.0, 1.0, 1.0, 1.0];
  shaderInfo.phongShader.mMatrix = mMatrix;
  setParams(true, false);
  drawSquare(color, shaderInfo.phongShader, 3, 0, 0);
  mMatrix = popMatrix(matrixStack);

  // Bottom side of cube
  pushMatrix(matrixStack, mMatrix);

  gl.activeTexture(gl.TEXTURE3);
  gl.bindTexture(gl.TEXTURE_2D, negy);
   
   // transformations
  mMatrix = mat4.translate(mMatrix, [0, -99.5, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(-90), [1, 0, 0]);
  mMatrix = mat4.scale(mMatrix, [200, 200, 200]);
  color = [0.0, 1.0, 1.0, 1.0];
  shaderInfo.phongShader.mMatrix = mMatrix;
  setParams(true, false);
  drawSquare(color, shaderInfo.phongShader, 3, 0, 0);
  mMatrix = popMatrix(matrixStack);
}

function drawPhongShaderProgram() {
  

  // setup shader code
  gl.useProgram(PhongShaderProgram);

  //enable the attribute arrays
  gl.enableVertexAttribArray(shaderInfo.phongShader.aPositionLocation);
  // enable the normal arrays
  gl.enableVertexAttribArray(shaderInfo.phongShader.normalPositionLocation);
    // enable the texture arrays
  gl.enableVertexAttribArray(shaderInfo.phongShader.aTextureLocation);

  // set up the view matrix, multiply into the modelview matrix
  mat4.identity(shaderInfo.phongShader.vMatrix);

  shaderInfo.phongShader.vMatrix = mat4.lookAt(eyePos, COI, viewUp, shaderInfo.phongShader.vMatrix);

  //set up perspective projection matrix
  mat4.identity(shaderInfo.phongShader.pMatrix);
  mat4.perspective(40, 1.0, 0.1, 1000, shaderInfo.phongShader.pMatrix);

  //set up the model matrix
  mMatrix = mat4.create();
  mat4.identity(mMatrix);
  
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.rotate(mMatrix, degToRad(degree0), [0, 1, 0]);
  mMatrix = mat4.rotate(mMatrix, degToRad(degree1), [1, 0, 0]);
  mMatrix = mat4.translate(mMatrix, [0, 0, 0]);
  
  drawSkyBox();
  
  mMatrix = mat4.translate(mMatrix, [0, -5, 0]);
  mMatrix = mat4.scale(mMatrix, [50, 50, 50]);
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.scale(mMatrix, [0.1,0.1, 0.1]);
  drawTable();
  mMatrix = popMatrix(matrixStack);

  // transformations applied here on model matrix
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [0.08, 0.071, 0.08]);
  mMatrix = mat4.rotate(mMatrix, degToRad(-5), [0, 1, 0]);
  mMatrix = mat4.scale(mMatrix, [0.03,0.03, 0.03]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  gl.activeTexture(gl.TEXTURE0);
  gl.bindTexture(gl.TEXTURE_2D, rcubeTex);
  shaderInfo.phongShader.mMatrix = mMatrix;
  setParams(true, false);
  drawCube(getColor(0,171,31), shaderInfo.phongShader, 0, 1, 0);
  mMatrix = popMatrix(matrixStack);

  gl.activeTexture(gl.TEXTURE1);
  gl.bindTexture(gl.TEXTURE_CUBE_MAP, cubeMapTex);

  // transformations applied here on model matrix
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [0.01, 0.0785, 0.1]);
  mMatrix = mat4.scale(mMatrix, [0.022,0.022, 0.022]);
//   mMatrix = mat4.rotate(mMatrix, degToRad(180), [0, 0, 1]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  setParams(false, false);
  drawSphere(getColor(101,200,76), shaderInfo.phongShader, 0, 1, 0.4);
  mMatrix = popMatrix(matrixStack);
  
  // transformations applied here on model matrix
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [0.11, 0.0748, -0.05]);
  mMatrix = mat4.scale(mMatrix, [0.02,0.02, 0.02]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  setParams(false, false);
  drawSphere(getColor(63,52,95), shaderInfo.phongShader, 0, 1, 0.4);
  mMatrix = popMatrix(matrixStack);

  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [-0.01, 0.1, -0.05]);
  mMatrix = mat4.scale(mMatrix, [0.12,0.12, 0.12]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  setParams(false, false);
  drawTeapot();
  mMatrix = popMatrix(matrixStack);
  
  // transformations applied here on model matrix
  pushMatrix(matrixStack, mMatrix);
  mMatrix = mat4.translate(mMatrix, [-0.09, 0.1, 0.06]);
  mMatrix = mat4.rotate(mMatrix, degToRad(30), [0, 1, 0]);
  mMatrix = mat4.scale(mMatrix, [0.05,0.08, 0.05]);
  shaderInfo.phongShader.mMatrix = mMatrix;
  setParams(false, true);
  drawCube(getColor(109,42,16), shaderInfo.phongShader, 0, 1, 1);
  mMatrix = popMatrix(matrixStack);

  mMatrix = popMatrix(matrixStack);
}

function drawScene() {

  // stop the current loop of animation
  if (animation) {
    window.cancelAnimationFrame(animation);
  }

  var animate = function () {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0.8274, 0.933, 0.8274, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    
    rot += 0.1;
    eyePos[2] = radius * Math.cos(degToRad(rot));
    eyePos[0] = radius * Math.sin(degToRad(rot));

    drawPhongShaderProgram();

    animation = window.requestAnimationFrame(animate);
  };

  animate();
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
    event.layerX <= canvas.width &&
    event.layerX >= 0 &&
    event.layerY <= canvas.height &&
    event.layerY >= 0
  ) {
    var mouseX = event.clientX;
    var diffX1 = mouseX - prevMouseX;
    prevMouseX = mouseX;
    degree0 = degree0 + diffX1 / 5;

    var mouseY = canvas.height - event.clientY;
    var diffY2 = mouseY - prevMouseY;
    prevMouseY = mouseY;
    degree1 = degree1 - diffY2 / 5;

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
  canvas = document.getElementById("assignment3");
  document.addEventListener("mousedown", onMouseDown, false);
  
  // initialize WebGL
  initGL(canvas);
  // enable depth test
  gl.enable(gl.DEPTH_TEST);
  
  PhongShaderProgram = initShaders(vertexShaderCode_PhongShading, fragShaderCode_PhongShading);
  
  shaderInfo.phongShader = {
    normalInfoRequired: true,
    aPositionLocation: gl.getAttribLocation(PhongShaderProgram, "aPosition"),
    normalPositionLocation: gl.getAttribLocation(PhongShaderProgram, "normalPosition"),
    aTextureLocation: gl.getAttribLocation(PhongShaderProgram, "textureCoords"),
    aTextureUnit: gl.getUniformLocation(PhongShaderProgram, "uTexture"),
    aReflectivity: gl.getUniformLocation(PhongShaderProgram, "reflectivity"),
    lookup2DTexLoc: gl.getUniformLocation(PhongShaderProgram, "lookup2DTex"),
    etaLoc: gl.getUniformLocation(PhongShaderProgram, "eta"),
    refractLoc: gl.getUniformLocation(PhongShaderProgram, "isRefract"),
    eyePosWorldLoc: gl.getUniformLocation(PhongShaderProgram, "eyePosWorld"),
    cubeMapLoc: gl.getUniformLocation(PhongShaderProgram, "cubeMap"),
    uMMatrixLocation: gl.getUniformLocation(PhongShaderProgram, "uMMatrix"),
    uVMatrixLocation: gl.getUniformLocation(PhongShaderProgram, "uVMatrix"),
    uPMatrixLocation: gl.getUniformLocation(PhongShaderProgram, "uPMatrix"),
    uNormalMatrixLocation: gl.getUniformLocation(PhongShaderProgram, "uNormalMatrix"),
    uwnMatrixLocation: gl.getUniformLocation(PhongShaderProgram, "uwNormalMatrix"),
    uColorLocation: gl.getUniformLocation(PhongShaderProgram, "objColor"),
    lightLocation: gl.getUniformLocation(PhongShaderProgram, "lightPos"),
    vMatrix: mat4.create(),
    pMatrix: mat4.create(),
    mMatrix: mat4.create(),
  };
  
  //initialize buffers for the square
  
  initObject();
  initSquareBuffer();
  initCubeBuffer();
  initSphereBuffer();
  
  rcubeTex = initTextures(rcubeFileLocation);
  woodTex = initTextures(woodTexPath);

  // initialise cubemap texture
  initCubeMapPath();
  initCubeMap();

  drawScene();
}