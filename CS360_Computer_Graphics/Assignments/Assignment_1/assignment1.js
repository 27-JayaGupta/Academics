////////////////////////////////////////////////////////////////////////
// A simple WebGL program to draw simple 2D shapes.
//

var gl;
var color;
var animation;
var degree_wind=0;
var degree_sun = 0;
var translate_amount = 0.005;
var translate_dis=0;
var translate_dir= 1;
var canvasWidth;
var canvasHeight;

var solidView = true;
var pointView = false;
var WireFrameView = false;

var matrixStack = [];

// mMatrix is called the model matrix, transforms objects
// from local object space to world space.
var mMatrix = mat4.create();
var uMMatrixLocation;

var triangleBuf;
var triangleIndexBuf;

var rightTriangleBuf;
var rightTriangleIndexBuf;
var rightTriangleBuf2;
var rightTriangleIndexBuf2;

var circleBuf;
var circleIndexBuf;

var sqVertexPositionBuffer;
var sqVertexIndexBuffer;

var aPositionLocation;
var uColorLoc;

const vertexShaderCode = `#version 300 es
in vec2 aPosition;
uniform mat4 uMMatrix;

void main() {
  gl_Position = uMMatrix*vec4(aPosition,0.0,1.0);
  gl_PointSize = 3.0;
}`;

const fragShaderCode = `#version 300 es
precision mediump float;
out vec4 fragColor;

uniform vec4 color;

void main() {
  fragColor = color;
}`;

function pointMode() {
  pointView = true;
  solidView = false;
  WireFrameView = false;
}

function wireFrameMode() {
  pointView = false;
  solidView = false;
  WireFrameView = true;
}

function solidMode() {
  pointView = false;
  solidView = true;
  WireFrameView = false;
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
    canvasHeight = canvas.height;
    canvasWidth = canvas.width;
  } catch (e) {}
  if (!gl) {
    alert("WebGL initialization failed");
  }
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

function drawSquare(color, mMatrix) {
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

  gl.uniform4fv(uColorLoc, color);
  
  if(solidView){
    // now draw the square
    gl.drawElements(
      gl.TRIANGLES,
      sqVertexIndexBuffer.numItems,
      gl.UNSIGNED_SHORT,
      0
    );
  }
  else if(pointView) {
    // now draw the square
    gl.drawElements(
      gl.POINTS,
      sqVertexIndexBuffer.numItems,
      gl.UNSIGNED_SHORT,
      0
    );
  }
  else if(WireFrameView) {
    // now draw the square
    gl.drawElements(
      gl.LINE_LOOP,
      sqVertexIndexBuffer.numItems,
      gl.UNSIGNED_SHORT,
      0
    );
  }
}

function initCircleBuffer() {
    // buffer for location points

    var circleVertices = [];
    var resolution = 64;
    var radius = 0.5;

    circleVertices[0] = 0;
    circleVertices[1] = 0;

    for(let i=0; i<resolution; i++) {
        var x = radius * Math.cos((2*Math.PI*i) / resolution);
        var y = radius * Math.sin((2*Math.PI*i) / resolution);
        circleVertices.push(x);
        circleVertices.push(y);
    }

    circleBuf = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, circleBuf);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(circleVertices), gl.STATIC_DRAW);
    circleBuf.itemSize = 2;
    circleBuf.numItems = 2*(resolution + 1);

    var circleIndices = [];
    for(let i=1; i<=resolution; i++) {
        circleIndices.push(0);
        circleIndices.push(i%(resolution+1));
        if((i+1)%(resolution+1) === 0)
            circleIndices.push(1);
        else
        circleIndices.push((i+1)%(resolution+1));
    }

    circleIndexBuf = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, circleIndexBuf);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(circleIndices), gl.STATIC_DRAW);
    circleIndexBuf.itemSize = 1;
    circleIndexBuf.numItems = (3*resolution);
}

function drawCircle(color, mMatrix) {
    gl.uniformMatrix4fv(uMMatrixLocation, false, mMatrix);
    gl.uniform4fv(uColorLoc, color);

    gl.bindBuffer(gl.ARRAY_BUFFER, circleBuf);
    gl.vertexAttribPointer(
        aPositionLocation,
        circleBuf.itemSize,
        gl.FLOAT,
        false,
        0,
        0
    );

    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, circleIndexBuf);
    
    if(solidView){
      gl.drawElements(
          gl.TRIANGLES,
          circleIndexBuf.numItems,
          gl.UNSIGNED_SHORT,
          0
      );
    }
    else if(pointView) {
      gl.drawElements(
        gl.POINTS,
        circleIndexBuf.numItems,
        gl.UNSIGNED_SHORT,
        0
      );
    }
    else if(WireFrameView) {
      gl.drawElements(
        gl.LINE_LOOP,
        circleIndexBuf.numItems,
        gl.UNSIGNED_SHORT,
        0
      );
    }
}

function initTriangleBuffer() {
  // buffer for point locations
  const triangleVertices = new Float32Array([0.0, 0.5, -0.5, -0.5, 0.5, -0.5]);
  triangleBuf = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, triangleBuf);
  gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
  triangleBuf.itemSize = 2;
  triangleBuf.numItems = 3;

  // buffer for point indices
  const triangleIndices = new Uint16Array([0, 1, 2]);
  triangleIndexBuf = gl.createBuffer();
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, triangleIndexBuf);
  gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, triangleIndices, gl.STATIC_DRAW);
  triangleIndexBuf.itemsize = 1;
  triangleIndexBuf.numItems = 3;
}

function drawTriangle(color, mMatrix) {
  gl.uniformMatrix4fv(uMMatrixLocation, false, mMatrix);

  // buffer for point locations
  gl.bindBuffer(gl.ARRAY_BUFFER, triangleBuf);
  gl.vertexAttribPointer(
    aPositionLocation,
    triangleBuf.itemSize,
    gl.FLOAT,
    false,
    0,
    0
  );

  // buffer for point indices
  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, triangleIndexBuf);

  gl.uniform4fv(uColorLoc, color);

  // now draw the square
  if(solidView) {
    gl.drawElements(
      gl.TRIANGLES,
      triangleIndexBuf.numItems,
      gl.UNSIGNED_SHORT,
      0
    );
  }
  else if(pointView) {
    gl.drawElements(
      gl.POINTS,
      triangleIndexBuf.numItems,
      gl.UNSIGNED_SHORT,
      0
    );
  }
  else if(WireFrameView) {
    gl.drawElements(
      gl.LINE_LOOP,
      triangleIndexBuf.numItems,
      gl.UNSIGNED_SHORT,
      0
    );
  }
}

function initRightTriangleBuffer() {
    // buffer for point locations
    const triangleVertices = new Float32Array([0.0, 0.0, 0.0, 0.5, 0.5, 0.0]);
    rightTriangleBuf = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, rightTriangleBuf);
    gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
    rightTriangleBuf.itemSize = 2;
    rightTriangleBuf.numItems = 3;
  
    // buffer for point indices
    const triangleIndices = new Uint16Array([0, 1, 2]);
    rightTriangleIndexBuf = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, rightTriangleIndexBuf);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, triangleIndices, gl.STATIC_DRAW);
    rightTriangleIndexBuf.itemsize = 1;
    rightTriangleIndexBuf.numItems = 3;
}
  
function drawRightTriangle(color, mMatrix) {
    gl.uniformMatrix4fv(uMMatrixLocation, false, mMatrix);

    // buffer for point locations
    gl.bindBuffer(gl.ARRAY_BUFFER, rightTriangleBuf);
    gl.vertexAttribPointer(
        aPositionLocation,
        rightTriangleBuf.itemSize,
        gl.FLOAT,
        false,
        0,
        0
    );

    // buffer for point indices
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, rightTriangleIndexBuf);

    gl.uniform4fv(uColorLoc, color);

    // now draw the square
    if(solidView){
      gl.drawElements(
          gl.TRIANGLES,
          rightTriangleIndexBuf.numItems,
          gl.UNSIGNED_SHORT,
          0
      );
    }
    else if(pointView) {
      gl.drawElements(
        gl.POINTS,
        rightTriangleIndexBuf.numItems,
        gl.UNSIGNED_SHORT,
        0
      );
    }
    else if(WireFrameView) {
      gl.drawElements(
        gl.LINE_LOOP,
        rightTriangleIndexBuf.numItems,
        gl.UNSIGNED_SHORT,
        0
      );
    }
}

function initRightTriangleBuffer2() {
    // buffer for point locations
    const triangleVertices = new Float32Array([0.0, 0.0, 0.0, 0.5, -0.5, 0.0]);
    rightTriangleBuf2 = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, rightTriangleBuf2);
    gl.bufferData(gl.ARRAY_BUFFER, triangleVertices, gl.STATIC_DRAW);
    rightTriangleBuf2.itemSize = 2;
    rightTriangleBuf2.numItems = 3;
  
    // buffer for point indices
    const triangleIndices = new Uint16Array([0, 1, 2]);
    rightTriangleIndexBuf2 = gl.createBuffer();
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, rightTriangleIndexBuf2);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, triangleIndices, gl.STATIC_DRAW);
    rightTriangleIndexBuf2.itemsize = 1;
    rightTriangleIndexBuf2.numItems = 3;
}
  
function drawRightTriangle2(color, mMatrix) {
    gl.uniformMatrix4fv(uMMatrixLocation, false, mMatrix);

    // buffer for point locations
    gl.bindBuffer(gl.ARRAY_BUFFER, rightTriangleBuf2);
    gl.vertexAttribPointer(
        aPositionLocation,
        rightTriangleBuf2.itemSize,
        gl.FLOAT,
        false,
        0,
        0
    );

    // buffer for point indices
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, rightTriangleIndexBuf2);

    gl.uniform4fv(uColorLoc, color);

    // now draw the square
    if(solidView) {
      gl.drawElements(
          gl.TRIANGLES,
          rightTriangleIndexBuf2.numItems,
          gl.UNSIGNED_SHORT,
          0
      );
    }
    else if(pointView) {
      gl.drawElements(
        gl.POINTS,
        rightTriangleIndexBuf2.numItems,
        gl.UNSIGNED_SHORT,
        0
      );
    }
    else if(WireFrameView) {
      gl.drawElements(
        gl.LINE_LOOP,
        rightTriangleIndexBuf2.numItems,
        gl.UNSIGNED_SHORT,
        0
      );
    }
}

function getColor(r, g, b) {
    color = [];
    color.push(r/255.0);
    color.push(g/255.0);
    color.push(b/255.0);
    color.push(1.0);
    
    return color;
}

function drawHouse(mMatrix) {
    
    // rooftop of house
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.scale(mMatrix, [0.5, 0.3, 1]);
    drawSquare(getColor(255, 102, 0), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.25, 0, 0]);
    mMatrix = mat4.scale(mMatrix, [0.3, 0.3, 1]);
    drawTriangle(getColor(255, 102, 0), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.25, 0, 0]);
    mMatrix = mat4.scale(mMatrix, [0.3, 0.3, 1]);
    drawTriangle(getColor(255, 102, 0), mMatrix);
    mMatrix = popMatrix(matrixStack);
    
    // body of house
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0, -0.31, 0]);
    mMatrix = mat4.scale(mMatrix, [0.6, 0.32, 1]);
    drawSquare(getColor(204,204,204), mMatrix);
    mMatrix = popMatrix(matrixStack);

    // door of house
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0, -0.37, 0]);
    mMatrix = mat4.scale(mMatrix, [0.1, 0.2, 1]);
    drawSquare(getColor(221,181,61), mMatrix);
    mMatrix = popMatrix(matrixStack);

    // window of house
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.2, -0.25, 0]);
    mMatrix = mat4.scale(mMatrix, [0.1, 0.1, 1]);
    drawSquare(getColor(221,181,61), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.2, -0.25, 0]);
    mMatrix = mat4.scale(mMatrix, [0.1, 0.1, 1]);
    drawSquare(getColor(221,181,61), mMatrix);
    mMatrix = popMatrix(matrixStack);
}

function drawWindMill(mMatrix, degree_wind) {

    // stand of windmill
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0, -0.45, 0]);
    mMatrix = mat4.scale(mMatrix, [0.06, 0.9, 1]);
    drawSquare(getColor(51,51,51), mMatrix);
    mMatrix = popMatrix(matrixStack);

    // wings
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.rotate(mMatrix,  degToRad(10+degree_wind), [0, 0, 1]);
    mMatrix = mat4.translate(mMatrix, [0.005, -0.2, 0]);
    mMatrix = mat4.scale(mMatrix, [0.13, 0.5, 1]);
    drawTriangle(getColor(179,179,57), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.rotate(mMatrix,  degToRad(100+degree_wind), [0, 0, 1]);
    mMatrix = mat4.translate(mMatrix, [0.005, -0.2, 0]);
    mMatrix = mat4.scale(mMatrix, [0.13, 0.5, 1]);
    drawTriangle(getColor(179,179,57), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.rotate(mMatrix,  degToRad(190+degree_wind), [0, 0, 1]);
    mMatrix = mat4.translate(mMatrix, [0.005, -0.2, 0]);
    mMatrix = mat4.scale(mMatrix, [0.13, 0.5, 1]);
    drawTriangle(getColor(179,179,57), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.rotate(mMatrix,  degToRad(280+degree_wind), [0, 0, 1]);
    mMatrix = mat4.translate(mMatrix, [0.005, -0.2, 0]);
    mMatrix = mat4.scale(mMatrix, [0.13, 0.5, 1]);
    drawTriangle(getColor(179,179,57), mMatrix);
    mMatrix = popMatrix(matrixStack);


    // black part of windmill
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.scale(mMatrix, [0.1, 0.1, 1]);
    drawCircle(getColor(0, 0, 0), mMatrix);
    mMatrix = popMatrix(matrixStack);
}

function drawBoat(mMatrix) {

    pushMatrix(matrixStack, mMatrix);

    if(translate_dis >= 1.8) {
        translate_dir = -1;
        translate_dis += (translate_amount * translate_dir);
    }
    else if(translate_dis <=-1.8) {
        translate_dir = 1;
        translate_dis += (translate_amount * translate_dir);
        
    }
    else {
        translate_dis += (translate_amount * translate_dir);
    }

    mMatrix = mat4.translate(mMatrix, [translate_dis, 0, 0]);

    // base of the boat
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.scale(mMatrix, [0.8, 0.3, 1]);
    drawSquare(getColor(204,204,204), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.4, 0, 0]);
    mMatrix = mat4.rotate(mMatrix, degToRad(180), [0, 0, 1]);
    mMatrix = mat4.scale(mMatrix, [0.3, 0.3, 1]);
    drawTriangle(getColor(204,204,204), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.4, 0, 0]);
    mMatrix = mat4.rotate(mMatrix, degToRad(180), [0, 0, 1]);
    mMatrix = mat4.scale(mMatrix, [0.3, 0.3, 1]);
    drawTriangle(getColor(204,204,204), mMatrix);
    mMatrix = popMatrix(matrixStack);

    // sail of boat
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.25, 0.55, 0]);
    mMatrix = mat4.rotate(mMatrix, degToRad(-90), [0, 0, 1]);
    mMatrix = mat4.scale(mMatrix, [0.6, 0.5, 1]);
    drawTriangle(getColor(255, 102, 0), mMatrix);
    mMatrix = popMatrix(matrixStack);

    // stand of boat black
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0, 0.55, 0]);
    mMatrix = mat4.scale(mMatrix, [0.03, 0.8, 1]);
    drawSquare(getColor(0,0,0), mMatrix);
    mMatrix = popMatrix(matrixStack);

    // stand of boat black(small slanting one)
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.19, 0.5, 0]);
    mMatrix = mat4.rotate(mMatrix, degToRad(-30), [0, 0, 1]);
    mMatrix = mat4.scale(mMatrix, [0.01, 0.8, 1]);
    drawSquare(getColor(0,0,0), mMatrix);
    mMatrix = popMatrix(matrixStack);
    
    mMatrix = popMatrix(matrixStack);
}

function drawTree(mMatrix) {

    // trunk of tree
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0, -1.2, 0]);
    mMatrix = mat4.scale(mMatrix, [0.2, 1, 1]);
    drawSquare(getColor(121,79,78), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0, -0.25, 0]);
    mMatrix = mat4.scale(mMatrix,[1,1,0]);
    drawTriangle(getColor(67,151,85), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0, -0.1, 0]);
    mMatrix = mat4.scale(mMatrix,[1.2,1,0]);
    drawTriangle(getColor(105,177,90), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.scale(mMatrix,[1.4,0.9,0]);
    drawTriangle(getColor(128,202,95), mMatrix);
    mMatrix = popMatrix(matrixStack);
}

function drawCar(mMatrix) {

    // wheels of car
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.5, -0.63, 0]);
    mMatrix = mat4.scale(mMatrix, [0.35, 0.35, 1]);
    drawCircle(getColor(0, 0, 0), mMatrix);
    mMatrix = popMatrix(matrixStack);
    
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.5, -0.63, 0]);
    mMatrix = mat4.scale(mMatrix, [0.25, 0.25, 1]);
    drawCircle(getColor(128,128,128), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.5, -0.63, 0]);
    mMatrix = mat4.scale(mMatrix, [0.35, 0.35, 1]);
    drawCircle(getColor(0, 0, 0), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.5, -0.63, 0]);
    mMatrix = mat4.scale(mMatrix, [0.25, 0.25, 1]);
    drawCircle(getColor(128,128,128), mMatrix);
    mMatrix = popMatrix(matrixStack);

    // rooftop of car
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0, -0.18, 0]);
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.scale(mMatrix, [0.6, 0.6, 1]);
    drawSquare(getColor(191,107,83), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.3, 0, 0]);
    mMatrix = mat4.scale(mMatrix, [0.6, 0.6, 1]);
    drawTriangle(getColor(191,107,83), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.3, 0, 0]);
    mMatrix = mat4.scale(mMatrix, [0.6, 0.6, 1]);
    drawTriangle(getColor(191,107,83), mMatrix);
    mMatrix = popMatrix(matrixStack);
    mMatrix = popMatrix(matrixStack);

    // body of car
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0, -0.35, 0]);
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.scale(mMatrix, [1.5, 0.4, 1]);
    drawSquare(getColor(55,126,222), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.75, 0, 0]);
    mMatrix = mat4.scale(mMatrix, [0.3, 0.4, 1]);
    drawTriangle(getColor(55,126,222), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.75, 0, 0]);
    mMatrix = mat4.scale(mMatrix, [0.3, 0.4, 1]);
    drawTriangle(getColor(55,126,222), mMatrix);
    mMatrix = popMatrix(matrixStack);
    mMatrix = popMatrix(matrixStack);
}

function drawMountain1(mMatrix) {

    // dark shade of mountain
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.scale(mMatrix,[3,1,0]);
    drawRightTriangle2(getColor(123,94,70), mMatrix);
    mMatrix = popMatrix(matrixStack);

    // left side of mountain light shade
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.scale(mMatrix,[2,1,0]);
    drawRightTriangle2(getColor(145,121,87), mMatrix);
    mMatrix = popMatrix(matrixStack);

    // right side of mountain light shade
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.scale(mMatrix,[4,1,0]);
    drawRightTriangle(getColor(145,121,87), mMatrix);
    mMatrix = popMatrix(matrixStack);
}

function drawMountain2(mMatrix) {
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.scale(mMatrix,[3,1,0]);
    drawTriangle(getColor(145,121,87), mMatrix);
    mMatrix = popMatrix(matrixStack);
}

function drawClouds(mMatrix) {

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.scale(mMatrix, [1, 0.6, 1]);
    drawCircle(getColor(255, 255, 255), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.5, -0.1, 1])
    mMatrix = mat4.scale(mMatrix, [0.8, 0.8, 1]);
    mMatrix = mat4.scale(mMatrix, [1, 0.6, 1]);
    drawCircle(getColor(255, 255, 255), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [1, -0.1, 1])
    mMatrix = mat4.scale(mMatrix, [0.6, 0.6, 1]);
    mMatrix = mat4.scale(mMatrix, [1, 0.6, 1]);
    drawCircle(getColor(255, 255, 255), mMatrix);
    mMatrix = popMatrix(matrixStack);
}

function drawGrass(mMatrix) {

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.55, -0.05, 1])
    mMatrix = mat4.scale(mMatrix, [0.5, 0.5, 1]);
    drawCircle(getColor(80,176,51), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.55, -0.05, 1])
    mMatrix = mat4.scale(mMatrix, [0.5, 0.5, 1]);
    drawCircle(getColor(42,100,25), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.scale(mMatrix, [1, 0.7, 1]);
    drawCircle(getColor(67,151,42), mMatrix);
    mMatrix = popMatrix(matrixStack);
}

function drawSun(mMatrix) {

    // rays
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.rotate(mMatrix,  degToRad(degree_sun), [0, 0, 1]);
    mMatrix = mat4.translate(mMatrix, [0.005, -0.2, 0]);
    mMatrix = mat4.scale(mMatrix, [0.01, 0.3, 1]);
    drawSquare(getColor(251,230,77), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.rotate(mMatrix,  degToRad(45+degree_sun), [0, 0, 1]);
    mMatrix = mat4.translate(mMatrix, [0.005, -0.2, 0]);
    mMatrix = mat4.scale(mMatrix, [0.01, 0.3, 1]);
    drawSquare(getColor(251,230,77), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.rotate(mMatrix,  degToRad(90+degree_sun), [0, 0, 1]);
    mMatrix = mat4.translate(mMatrix, [0.005, -0.2, 0]);
    mMatrix = mat4.scale(mMatrix, [0.01, 0.3, 1]);
    drawSquare(getColor(251,230,77), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.rotate(mMatrix,  degToRad(135+degree_sun), [0, 0, 1]);
    mMatrix = mat4.translate(mMatrix, [0.005, -0.2, 0]);
    mMatrix = mat4.scale(mMatrix, [0.01, 0.3, 1]);
    drawSquare(getColor(251,230,77), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.rotate(mMatrix,  degToRad(180+degree_sun), [0, 0, 1]);
    mMatrix = mat4.translate(mMatrix, [0.005, -0.2, 0]);
    mMatrix = mat4.scale(mMatrix, [0.01, 0.3, 1]);
    drawSquare(getColor(251,230,77), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.rotate(mMatrix,  degToRad(225+degree_sun), [0, 0, 1]);
    mMatrix = mat4.translate(mMatrix, [0.005, -0.2, 0]);
    mMatrix = mat4.scale(mMatrix, [0.01, 0.3, 1]);
    drawSquare(getColor(251,230,77), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.rotate(mMatrix,  degToRad(270+degree_sun), [0, 0, 1]);
    mMatrix = mat4.translate(mMatrix, [0.005, -0.2, 0]);
    mMatrix = mat4.scale(mMatrix, [0.01, 0.3, 1]);
    drawSquare(getColor(251,230,77), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.rotate(mMatrix,  degToRad(315+degree_sun), [0, 0, 1]);
    mMatrix = mat4.translate(mMatrix, [0.005, -0.2, 0]);
    mMatrix = mat4.scale(mMatrix, [0.01, 0.3, 1]);
    drawSquare(getColor(251,230,77), mMatrix);
    mMatrix = popMatrix(matrixStack);

    // sun mid part
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.rotate(mMatrix,  degToRad(degree_sun), [0, 0, 1]);
    mMatrix = mat4.scale(mMatrix, [0.45, 0.45, 1]);
    drawCircle(getColor(251,230,77), mMatrix);
    mMatrix = popMatrix(matrixStack);
}

function drawBird(mMatrix) {

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.scale(mMatrix, [0.05, 0.05, 1]);
    drawSquare(getColor(0, 0, 0), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.08, 0.05, 0]);
    mMatrix = mat4.rotate(mMatrix, degToRad(-20), [0,0,1]);
    mMatrix = mat4.scale(mMatrix, [0.15, 0.03, 1]);
    drawTriangle(getColor(0, 0, 0), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.08, 0.05, 0]);
    mMatrix = mat4.rotate(mMatrix, degToRad(20), [0,0,1]);
    mMatrix = mat4.scale(mMatrix, [0.15, 0.03, 1]);
    drawTriangle(getColor(0, 0, 0), mMatrix);
    mMatrix = popMatrix(matrixStack);
}

function drawSky() {
    mat4.identity(mMatrix);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0, 1, 1]);
    mMatrix = mat4.scale(mMatrix, [2, 2, 1]);
    drawSquare(getColor(128,202,250), mMatrix);
    mMatrix = popMatrix(matrixStack);
}

function drawLawn() {
    mat4.identity(mMatrix);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0, -1, 1]);
    mMatrix = mat4.scale(mMatrix, [2, 2, 1]);
    drawSquare(getColor(104,226,138), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.35,-0.6, 1]);
    mMatrix = mat4.rotate(mMatrix,  degToRad(50), [0, 0, 1]);
    mMatrix = mat4.scale(mMatrix,[1,1.5,0]);
    drawTriangle(getColor(120,177,72), mMatrix);
    mMatrix = popMatrix(matrixStack);
}

function drawRiver() {
    mat4.identity(mMatrix);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0, -0.2, 1]);
    mMatrix = mat4.scale(mMatrix, [2, 0.3, 1]);
    drawSquare(getColor(42,100,246), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.5, -0.2, 1]);
    mMatrix = mat4.scale(mMatrix, [0.3, 0.005, 1]);
    drawSquare(getColor(204,204,204), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.1, -0.15, 1]);
    mMatrix = mat4.scale(mMatrix, [0.3, 0.002, 1]);
    drawSquare(getColor(204,204,204), mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.7, -0.3, 1]);
    mMatrix = mat4.scale(mMatrix, [0.3, 0.004, 1]);
    drawSquare(getColor(204,204,204), mMatrix);
    mMatrix = popMatrix(matrixStack);
}

function drawBackground() {
    drawSky();
    drawLawn();
    drawRiver();
}
////////////////////////////////////////////////////////////////////////
function drawScene() {
  // stop the current loop of animation
  if (animation) {
    window.cancelAnimationFrame(animation);
  }

  var animate = function () {
    gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);
    gl.clearColor(255, 255, 255, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    mat4.identity(mMatrix);

    degree_wind -= 1;
    degree_sun += 0.5
    drawBackground();

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.66, 0, 1]);
    mMatrix = mat4.scale(mMatrix, [0.3, 0.3, 1]);
    drawMountain1(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.8, 0.08, 1]);
    mMatrix = mat4.scale(mMatrix, [0.2, 0.15, 1]);
    drawMountain2(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.scale(mMatrix, [0.5, 0.5, 1]);
    drawMountain1(mMatrix);
    mMatrix = popMatrix(matrixStack);
    
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.75, 0.57, 1]);
    mMatrix = mat4.scale(mMatrix, [0.35, 0.35, 1]);
    drawTree(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.4, 0.66, 1]);
    mMatrix = mat4.scale(mMatrix, [0.4, 0.4, 1]);
    drawTree(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.1, 0.5, 1]);
    mMatrix = mat4.scale(mMatrix, [0.3, 0.3, 1]);
    drawTree(mMatrix);
    mMatrix = popMatrix(matrixStack);

    // check this function, translation not working properly

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.9, 0.55, 0]);
    mMatrix = mat4.scale(mMatrix, [0.3, 0.3, 1]);
    drawClouds(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0, -0.2, 1]);
    mMatrix = mat4.scale(mMatrix, [0.4, 0.4, 1]);
    drawBoat(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.5, 0.1, 0]);
    mMatrix = mat4.scale(mMatrix, [0.7, 0.7, 1]);
    drawWindMill(mMatrix, degree_wind);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.5, 0.1, 0]);
    mMatrix = mat4.scale(mMatrix, [0.7, 0.7, 1]);
    drawWindMill(mMatrix, degree_wind);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.27, -0.65, 0]);
    mMatrix = mat4.scale(mMatrix, [0.2, 0.2, 1]);
    drawGrass(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.86, -0.65, 0]);
    mMatrix = mat4.scale(mMatrix, [0.2, 0.2, 1]);
    drawGrass(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.6, -0.4, 0]);
    mMatrix = mat4.scale(mMatrix, [0.7, 0.7, 1]);
    drawHouse(mMatrix);
    mMatrix = popMatrix(matrixStack);
    
    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.5, -0.77, 0]);
    mMatrix = mat4.scale(mMatrix, [0.2, 0.2, 1]);
    drawCar(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.98, -0.55, 0]);
    mMatrix = mat4.scale(mMatrix, [0.3, 0.3, 1]);
    drawGrass(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.1, -1, 0]);
    mMatrix = mat4.scale(mMatrix, [0.3, 0.2, 1]);
    drawGrass(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.8  , 0.8, 0]);
    mMatrix = mat4.scale(mMatrix, [0.5, 0.5, 1]);
    drawSun(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.1  , 0.7, 0]);
    mMatrix = mat4.scale(mMatrix, [0.6, 0.6, 1]);
    drawBird(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.35  , 0.75, 0]);
    mMatrix = mat4.scale(mMatrix, [0.4, 0.4, 1]);
    drawBird(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [0.2, 0.85, 0]);
    mMatrix = mat4.scale(mMatrix, [0.45, 0.45, 1]);
    drawBird(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.1, 0.9, 0]);
    mMatrix = mat4.scale(mMatrix, [0.2, 0.2, 1]);
    drawBird(mMatrix);
    mMatrix = popMatrix(matrixStack);

    pushMatrix(matrixStack, mMatrix);
    mMatrix = mat4.translate(mMatrix, [-0.25, 0.8, 0]);
    mMatrix = mat4.scale(mMatrix, [0.25, 0.25, 1]);
    drawBird(mMatrix);
    mMatrix = popMatrix(matrixStack);
    
    animation = window.requestAnimationFrame(animate);
  };

  animate();
}

// This is the entry point from the html
function webGLStart() {
  var canvas = document.getElementById("examplePrimitiveRender");
  initGL(canvas);
  shaderProgram = initShaders();

  //get locations of attributes declared in the vertex shader
  const aPositionLocation = gl.getAttribLocation(shaderProgram, "aPosition");

  uMMatrixLocation = gl.getUniformLocation(shaderProgram, "uMMatrix");

  //enable the attribute arrays
  gl.enableVertexAttribArray(aPositionLocation);

  uColorLoc = gl.getUniformLocation(shaderProgram, "color");

  initSquareBuffer();
  initTriangleBuffer();
  initRightTriangleBuffer();
  initRightTriangleBuffer2();
  initCircleBuffer();

  drawScene();
}
