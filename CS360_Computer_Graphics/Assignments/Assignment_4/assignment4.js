var canvas;

var sqVertexPositionBuffer;
var sqVertexTextureBuffer;
var sqVertexNormalBuffer;
var sqVertexIndexBuffer;

var matrixStack = [];
var mMatrix;
var onlyBackground = true;

var aPositionLocation;
var aTextureLocation;
var uMMatrixLocation;
var uTextureFLocation;
var uTextureBLocation;
var onlyBackgroundLocation;
var textureSizeLocation;
var contrastLocation;
var brigtnessLocation;
var isKernelLocation;
var kernelLocation;
var kernelWeightLocation;
var isGradientLocation;

var isAlphaBlendedLocation;
var isGrayLocation;
var isSepiaLocation;

var imgFile1;
var imgFile2;

// Shader programs for different types of shading
var shaderProgram;

var backgroundTex;
var foregroundTex;

var isAlphaBlended = false;
var isGray = false;
var isSepia = false;
var contrast = 0.0;
var brightness = 0.0;
var isKernel = false;
var isGradient = false;

var kernel = [
    0, 0, 0,
    0, 1, 0,
    0, 0, 0
  ];
var kernelWeight  = 1.0;

// Vertex shader code for Phong Shading
const vertexShaderCode = `#version 300 es
in vec3 aPosition;
in vec2 textureCoords;

uniform mat4 uMMatrix;

out vec2 fragTexCoord;

void main() {

  fragTexCoord = textureCoords;
  gl_Position = uMMatrix*vec4(aPosition,1.0);
  gl_PointSize = 3.0;
}`;

// Fragment shader code for Phong Shading
const fragShaderCode = `#version 300 es

precision highp float;

in vec2 fragTexCoord;

uniform sampler2D uTextureB;
uniform sampler2D uTextureF;
uniform bool onlyBackground;
uniform vec2 u_textureSize;
uniform bool isAlphaBlended;
uniform bool isGray;
uniform bool isSepia;
uniform float contrast;
uniform float brightness;
uniform bool isKernel;
uniform bool isGradient;
uniform float u_kernel[9];
uniform float u_kernelWeight;

out vec4 fragColor;

void main() {

    vec4 plainColor;
    vec4 procColor;
    float pixelAmt = 7.0;

    if(onlyBackground) {
      if(isKernel) {
        // do kernel manipulation only on background image
        vec2 onePixel = vec2(1.0, 1.0) / u_textureSize;
        vec4 colorSum =
          texture(uTextureB, fragTexCoord + pixelAmt * onePixel * vec2(-1, -1)) * u_kernel[0] +
          texture(uTextureB, fragTexCoord + pixelAmt * onePixel * vec2( 0, -1)) * u_kernel[1] +
          texture(uTextureB, fragTexCoord + pixelAmt * onePixel * vec2( 1, -1)) * u_kernel[2] +
          texture(uTextureB, fragTexCoord + pixelAmt * onePixel * vec2(-1,  0)) * u_kernel[3] +
          texture(uTextureB, fragTexCoord + pixelAmt * onePixel * vec2( 0,  0)) * u_kernel[4] +
          texture(uTextureB, fragTexCoord + pixelAmt * onePixel * vec2( 1,  0)) * u_kernel[5] +
          texture(uTextureB, fragTexCoord + pixelAmt * onePixel * vec2(-1,  1)) * u_kernel[6] +
          texture(uTextureB, fragTexCoord + pixelAmt * onePixel * vec2( 0,  1)) * u_kernel[7] +
          texture(uTextureB, fragTexCoord + pixelAmt * onePixel * vec2( 1,  1)) * u_kernel[8] ;
      
        // Divide the sum by the weight but just use rgb
        // we'll set alpha to 1.0
        plainColor = vec4((colorSum/ u_kernelWeight).rgb, 1.0);
      }
      else if(isGradient) {
        vec2 onePixel = vec2(1.0, 1.0) / u_textureSize;
        vec4 up = texture(uTextureB, fragTexCoord + pixelAmt * onePixel * vec2(0, -1));
        vec4 down = texture(uTextureB, fragTexCoord + pixelAmt * onePixel * vec2(0, 1));
        vec4 left = texture(uTextureB, fragTexCoord + pixelAmt * onePixel * vec2(1, 0));
        vec4 right = texture(uTextureB, fragTexCoord + pixelAmt * onePixel * vec2(-1, 0));
        vec4 dy = (up-down)*0.5;
        vec4 dx = (right-left)*0.5;
        vec4 gradMag = sqrt(dx*dx+dy*dy);
        
        plainColor = vec4(gradMag.rgb+0.1, 1.0);
      }
      else {
        plainColor = texture(uTextureB, fragTexCoord);
      }
    }
    else {
      vec4 backgroundColor = texture(uTextureB, fragTexCoord);
      vec4 foregroundColor = texture(uTextureF, fragTexCoord);
      
      // get alpha blended color here
      plainColor = mix(backgroundColor, foregroundColor, foregroundColor.a);
    }

    if(isGray) {
      float gc = dot(vec3(0.2126, 0.7152, 0.0722), plainColor.rgb) ;
      procColor = vec4(gc, gc, gc, plainColor.a);
    }
    else if(isSepia) {
      float sr = dot(vec3(0.393, 0.769, 0.189), plainColor.rgb);
      float sg = dot(vec3( 0.349, 0.686, 0.168), plainColor.rgb);
      float sb = dot(vec3(0.272, 0.534, 0.131), plainColor.rgb);

      procColor = vec4(sr, sg, sb, plainColor.a);
    }
    else {
      procColor = plainColor;
    }

    fragColor = vec4(0.5 + (contrast + 1.0) * (procColor.rgb - 0.5), procColor.a);
    fragColor = vec4(fragColor.rgb + brightness, fragColor.a);
    // fragColor = procColor;
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

  // setup shader code
  gl.useProgram(shaderProgram);
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

function initGL(canvas) {
  try {
    gl = canvas.getContext("webgl2", {preserveDrawingBuffer: true}); // the graphics webgl2 context
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
    gl.RGBA, // internal format
    gl.RGBA, // format
    gl.UNSIGNED_BYTE, // type of data
    texture.image // array or <img>
  );

  gl.generateMipmap(gl.TEXTURE_2D);
  // Set the parameters so we can render any size image.
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
  gl.texParameteri(
    gl.TEXTURE_2D,
    gl.TEXTURE_MIN_FILTER,
    gl.LINEAR_MIPMAP_LINEAR
  );
  
  // set the size of the image
  gl.uniform2f(textureSizeLocation, texture.image.width, texture.image.height);

  drawScene();
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
    1, 1, 0, 1, 0, 0, 1, 0
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

function drawSquare() {
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

  gl.bindBuffer(gl.ARRAY_BUFFER, sqVertexTextureBuffer);
  gl.vertexAttribPointer(
    aTextureLocation,
    sqVertexTextureBuffer.itemSize,
    gl.FLOAT,
    false,
    0,
    0
  );

  gl.uniform1i(uTextureBLocation, 0);
  gl.uniform1i(uTextureFLocation, 1);
  gl.uniform1i(onlyBackgroundLocation, onlyBackground);
  gl.uniform1i(isAlphaBlendedLocation, isAlphaBlended);
  gl.uniform1i(isGrayLocation, isGray);
  gl.uniform1i(isSepiaLocation, isSepia);
  gl.uniform1f(contrastLocation, contrast);
  gl.uniform1f(brightnessLocation, brightness);
  gl.uniform1i(isKernelLocation, isKernel);
  gl.uniformMatrix4fv(uMMatrixLocation, false, mMatrix);
  gl.uniform1i(isGradientLocation, isGradient);
  gl.uniform1fv(kernelLocation, kernel);
  gl.uniform1f(kernelWeightLocation, kernelWeight);

  // buffer for point indices
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, sqVertexIndexBuffer);

  gl.drawElements(
    gl.TRIANGLES,
    sqVertexIndexBuffer.numItems,
    gl.UNSIGNED_SHORT,
    0
  );
}

function drawScene() {
    gl.viewport(0, 0, canvas.width, canvas.height);
    gl.clearColor(0, 0, 0, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);
    
    

    //set up the model matrix
    mMatrix = mat4.create();
    mat4.identity(mMatrix);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, backgroundTex);
    gl.activeTexture(gl.TEXTURE1);
    gl.bindTexture(gl.TEXTURE_2D, foregroundTex);

    // draw the square
    mMatrix = mat4.scale(mMatrix, [2, 2, 2]);
    drawSquare();
}

function uploadBackground() {
  onlyBackground = true;
  backgroundTex  = initTextures(imgFile1);
}

function uploadAlphaForeground() {
  onlyBackground = false;
  backgroundTex  = initTextures(imgFile1);
  foregroundTex  = initTextures(imgFile2);
}

function regImageBackground(fileElem) {
    // Handle changes to the selected file.
    fileElem.onchange = function (changeEvent) {
      // Guard against invalid input (no files).
      if (!changeEvent?.target?.files?.length) return;
      
      // Read the file as a blob.
      const firstFile = changeEvent.target.files[0];
      imgFile1 = URL.createObjectURL(firstFile);
      document.getElementById('background').checked = false;
    };
}

function regImageForeground(fileElem) {
  // Handle changes to the selected file.
  fileElem.onchange = function (changeEvent) {
    // Guard against invalid input (no files).
    if (!changeEvent?.target?.files?.length) return;
    document.getElementById('alpha').checked = false;
    // Read the file as a blob.
    const firstFile = changeEvent.target.files[0];
    imgFile2 = URL.createObjectURL(firstFile);
  };
}

function change_to_default() {
    isGray = false;
    isSepia = false;
    isGradient = false;
    isKernel = false;
    
}

function doGray() {
    change_to_default();
  let checkbox = document.getElementById('gray');
  if (checkbox.checked) {
    isGray = true;
  } else {
    isGray = false;
  }

  drawScene();
}

function doSepia() {
    change_to_default();
  let checkbox = document.getElementById('sepia');
  if (checkbox.checked) {
    isSepia = true;
  } else {
    isSepia = false;
  }

  drawScene();
}

function doSmooth() {
    change_to_default();
  let checkbox = document.getElementById('smooth');
  if (checkbox.checked) {
    isKernel = true;
    kernel = [
      1, 1, 1,
      1, 1, 1,
      1, 1, 1
    ];
    kernelWeight = 9.0;

  } else {
    isKernel = false;
    kernel = [
        0, 0, 0,
        0, 1, 0,
        0, 0, 0
      ];
    kernelWeight = 1.0;
  }

  drawScene();
}

function doSharpen() {
    change_to_default();
  let checkbox = document.getElementById('sharpen');
  if (checkbox.checked) {
    isKernel = true;
    kernel = [
      0, -1, 0,
      -1, 5, -1,
      0, -1, 0
    ];

    kernelWeight = 1.0;
  } else {
    isKernel = false;
    kernel = [
        0, 0, 0,
        0, 1, 0,
        0, 0, 0
      ];
    kernelWeight = 1.0;
  }

  drawScene();
}

function doLaplacian () {
    change_to_default();
  let checkbox = document.getElementById('laplacian');
  if (checkbox.checked) {
    isKernel = true;
    kernel = [
      0, -1, 0,
      -1, 4, -1,
      0, -1, 0
    ];

    kernelWeight = 1.0;
  } else {
    isKernel = false;
    kernel = [
        0, 0, 0,
        0, 1, 0,
        0, 0, 0
      ];
    kernelWeight = 1.0; 
  }

  drawScene();
}

function doGradient () {
  change_to_default();
  let checkbox = document.getElementById('gradient');
  if (checkbox.checked) {
    isGradient = true;
  } else {
    isGradient = false;
  }

  drawScene();
}

function resetCanvas() {
  isGray = false;
  isSepia = false;
  isKernel = false;
  isGradient = false;
  isAlphaBlended = false;
  contrast = 0.0;
  brightness = 0.0;
  onlyBackground = true;

  document.getElementById('gray').checked = false;
  document.getElementById('sepia').checked = false;
  document.getElementById('smooth').checked = false;
  document.getElementById('sharpen').checked = false;
  document.getElementById('laplacian').checked = false;
  document.getElementById('gradient').checked = false;
  document.getElementById('alpha').checked = false;
  document.getElementById('contrast').value = 0;
  document.getElementById('brightness').value = 0;

  drawScene();
}

function saveScreenshot() {  
  var image = canvas.toDataURL("image/png");
  var link = document.createElement('a');
  link.download = "my-image.png";
  link.href = image;
  link.click();
}

// This is the entry point from the html
function webGLStart() {
    canvas = document.getElementById("assignment4");

    // initialize WebGL
    initGL(canvas);
    gl.enable(gl.BLEND);
    gl.disable(gl.DEPTH_TEST);

    shaderProgram = initShaders(vertexShaderCode, fragShaderCode);

    initSquareBuffer();

    regImageBackground(document.getElementById("img1"));
    regImageForeground(document.getElementById("img2"))

    aPositionLocation = gl.getAttribLocation(shaderProgram, "aPosition");
    aTextureLocation = gl.getAttribLocation(shaderProgram, "textureCoords");
    uMMatrixLocation = gl.getUniformLocation(shaderProgram, "uMMatrix");
    uTextureBLocation = gl.getUniformLocation(shaderProgram, "uTextureB");
    uTextureFLocation = gl.getUniformLocation(shaderProgram, "uTextureF");
    onlyBackgroundLocation = gl.getUniformLocation(shaderProgram, "onlyBackground");
    textureSizeLocation = gl.getUniformLocation(shaderProgram, "u_textureSize");
    isAlphaBlendedLocation = gl.getUniformLocation(shaderProgram, "isAlphaBlended");
    isGrayLocation = gl.getUniformLocation(shaderProgram, "isGray");
    isSepiaLocation = gl.getUniformLocation(shaderProgram, "isSepia");
    contrastLocation = gl.getUniformLocation(shaderProgram, "contrast");
    brightnessLocation = gl.getUniformLocation(shaderProgram, "brightness");
    isKernelLocation = gl.getUniformLocation(shaderProgram, "isKernel");
    kernelLocation = gl.getUniformLocation(shaderProgram, "u_kernel[0]");
    kernelWeightLocation = gl.getUniformLocation(shaderProgram, "u_kernelWeight");
    isGradientLocation = gl.getUniformLocation(shaderProgram, "isGradient");  

    //enable the attribute arrays
    gl.enableVertexAttribArray(aPositionLocation);
    // enable the texture arrays
    gl.enableVertexAttribArray(aTextureLocation);

    var sliderContrast = document.getElementById("contrast");

    // Update the current slider value (each time you drag the slider handle)
    sliderContrast.oninput = function() {
      contrast = this.value;
      drawScene();
    }

    var sliderBrightness = document.getElementById("brightness");

    // Update the current slider value (each time you drag the slider handle)
    sliderBrightness.oninput = function() {
      brightness = this.value;
      drawScene();
    }

    drawScene();
}