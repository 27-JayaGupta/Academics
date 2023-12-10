var canvas;

var sqVertexPositionBuffer;
var sqVertexIndexBuffer;

var aPositionLocation;
var lightPosPosition;
var cameraPosPosition;
var bounceLimitPosition;
var canvWidthPosition;
var canvHeightPosition;
var isShadowPosition;
var isReflectionPosition;

// Shader programs for different types of shading
var shaderProgram;

var isShadow = false;
var isReflection = false;

var lightPos = [0, 200, 100];
var bounceLimit = 0;

// Vertex shader code for Phong Shading
const vertexShaderCode = `#version 300 es
in vec3 aPosition;

void main() {
  gl_Position = vec4(aPosition,1.0);
  gl_PointSize = 3.0;
}`;

// Fragment shader code for Phong Shading
const fragShaderCode = `#version 300 es

precision highp float;

uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform int bounceLimit;
uniform float canvWidth;
uniform float canvHeight;
uniform bool isShadow;
uniform bool isReflection;

out vec4 fragColor;

const float bias = 0.0001;

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Material {
    vec3 color; // solid color
    float refectivity;  // how much contribution from reflected ray
    float shininess; // how much contribution from specular component
};

struct Sphere {
    vec3 center;
    float radius;
    Material material;
};

struct HitPoint {
    vec3 position;
    float t;
    vec3 normal;
    Material material;
};

Sphere spheres[4]; // all spheres in the scene

// functions
Ray initRay() {
    Ray ray;
    // create the ray for current frag
    ray.origin = cameraPos;

    // direction is through each screen fragment in negative z direction
    vec2 screenPos = gl_FragCoord.xy/vec2(canvWidth, canvHeight);
    ray.direction = normalize(vec3(screenPos * 2.0 - 1.0, -1.0));
    return ray;
}

void initSpheres() {
    // initialize spheres
    spheres[0].center = vec3(0.0, -9.0, 0.0);
    spheres[0].radius = 9.0;
    spheres[0].material.color = vec3(0.0, 0.0, 0.0);
    spheres[0].material.refectivity = 0.4;
    spheres[0].material.shininess = 0.5;

    // Right
    spheres[1].center = vec3(1.0, 0.3, 3.5);
    spheres[1].radius = 0.5;
    spheres[1].material.color = vec3(0.0, 0.0, 1.0);
    spheres[1].material.refectivity = 0.4;
    spheres[1].material.shininess = 70.0;

    // Left
    spheres[2].center = vec3(-1.0, 0.3, 3.5);
    spheres[2].radius = 0.5;
    spheres[2].material.color = vec3(0.0, 1.0, 0.0);
    spheres[2].material.refectivity = 0.4;
    spheres[2].material.shininess = 25.0;

    // Center
    spheres[3].center = vec3(0.0, 0.8, 2.3);
    spheres[3].radius = 1.0;
    spheres[3].material.color = vec3(1.0, 0.0, 0.0);
    spheres[3].material.refectivity = 0.4;
    spheres[3].material.shininess = 10.0;
}

float intersectSphere(in Ray ray, in Sphere sphere) {
    vec3 oc = ray.origin - sphere.center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - sphere.radius * sphere.radius;
    float discriminant = b * b - 4.0 * a * c;
    if (discriminant < 0.0) {
        return -1.0;
    } else {
        return (-b - sqrt(discriminant)) / (2.0 * a);
    }
}

bool intersectScene(in Ray ray, inout HitPoint hitPoint) {
    bool hit = false;
    float t = 100000.0;
    for (int i = 0; i < 4; i++) {
        float t0 = intersectSphere(ray, spheres[i]);
        if (t0 > 0.0 && t0 < t) {
            t = t0;
            hitPoint.position = ray.origin + ray.direction * t;
            hitPoint.t = t;
            hitPoint.normal = normalize(hitPoint.position - spheres[i].center);
            hitPoint.material = spheres[i].material;
            hit = true;
        }
    }
    return hit;
}

vec3 computeColor(in HitPoint hitPoint) {
    float Kd = 1.0;
    float Ks = 1.0;
    float Ka = 0.3;
    float intensity = 1.0;

    vec3 color = vec3(0.0, 0.0, 0.0);

    // Compute phong shading color
    vec3 lightDir = normalize(lightPos - hitPoint.position);
    float cos_theta = dot(hitPoint.normal, lightDir);
    cos_theta = clamp(cos_theta, 0.0, 1.0);
  
    vec3 diffuseColor = Kd * cos_theta * hitPoint.material.color;

    vec3 viewDir = normalize(cameraPos - hitPoint.position);
    vec3 reflection = normalize(reflect(-lightDir, hitPoint.normal));

    float cos_phi = dot(viewDir, reflection);
    cos_phi = clamp(cos_phi, 0.0, 1.0);
    cos_phi = pow(cos_phi, hitPoint.material.shininess);
    vec3 specularColor = Ks * cos_phi * vec3(1.0, 1.0, 1.0);

    vec3 ambientColor = Ka * hitPoint.material.color;

    // Specular color is from the reflection of the light from the surface. If light is behind the surface, no color will be seen. Only black object
    if (cos_theta < 0.0) {
        diffuseColor = vec3(0.0, 0.0, 0.0);
        specularColor = vec3(0.0, 0.0, 0.0);
    }

    color = intensity * (diffuseColor + specularColor + ambientColor);

    if(isShadow) {
        // Check whether in shadow or not
        Ray shadowRay;
        shadowRay.origin = hitPoint.position + hitPoint.normal * bias;
        shadowRay.direction = normalize(lightPos - hitPoint.position);

        for(int i = 0; i < 4; i++) {
            float t0 = intersectSphere(shadowRay, spheres[i]);
            if (t0 > 0.0) {
                // hit point found
                color = 0.85 * color;
                break;
            }
        }
    }

    return color;
}

void main() {
    
    initSpheres();

    HitPoint hitPoint;
    Ray ray = initRay();

    vec3 color = vec3(0.0, 0.0, 0.0);
    bool hit = intersectScene(ray, hitPoint);

    if(hit) {
        color = computeColor(hitPoint);

        if(isReflection) {
            // Take into account reflection
            for(int i = 0; i < bounceLimit; i++) {
                Ray reflectedRay;
                reflectedRay.origin = hitPoint.position + hitPoint.normal * bias;
                reflectedRay.direction = reflect(ray.direction, hitPoint.normal);

                hit = intersectScene(reflectedRay, hitPoint);

                if(hit) {
                    vec3 reflectedColor = computeColor(hitPoint);
                    color = mix(color, reflectedColor, hitPoint.material.refectivity);

                    ray = reflectedRay;
                }
                else {
                    color = mix(color, vec3(0.0,0.0,0.0), 0.1);
                    break;
                }
            }
        }
    }

    fragColor = vec4(color, 1.0);
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

function initSquareBuffer() {
  // buffer for point locations
  const sqVertices = new Float32Array([
    1.0, 1.0, 0.0, -1.0, 1.0, 0.0,-1.0, -1.0, 0.0, 1.0, -1.0, 0.0,
  ]);
  sqVertexPositionBuffer = gl.createBuffer();
  gl.bindBuffer(gl.ARRAY_BUFFER, sqVertexPositionBuffer);
  gl.bufferData(gl.ARRAY_BUFFER, sqVertices, gl.STATIC_DRAW);
  sqVertexPositionBuffer.itemSize = 3;
  sqVertexPositionBuffer.numItems = 4;

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

    gl.uniform1i(bounceLimitPosition, bounceLimit);
    gl.uniform3fv(lightPosPosition, lightPos);
    gl.uniform3fv(cameraPosPosition, [0, 0, 5]);
    gl.uniform1f(canvWidthPosition, canvas.width);
    gl.uniform1f(canvHeightPosition, canvas.height);
    gl.uniform1i(isShadowPosition, isShadow);
    gl.uniform1i(isReflectionPosition, isReflection);

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
    
    drawSquare();
}

function phong() {
    isShadow = false;
    isReflection = false;
    drawScene();
}

function phongShadow() {
    isShadow = true;
    isReflection = false;
    drawScene();
}

function phongReflection() {
    isShadow = false;
    isReflection = true;
    if(!bounceLimit)
      bounceLimit = 1;
    drawScene();
}

function phongShadowReflection() {
    isShadow = true;
    isReflection = true;
    if(!bounceLimit)
      bounceLimit = 1;
    drawScene();
}

// This is the entry point from the html
function webGLStart() {
    canvas = document.getElementById("assignment5");

    // initialize WebGL
    initGL(canvas);
    gl.enable(gl.BLEND);
    gl.disable(gl.DEPTH_TEST);

    shaderProgram = initShaders(vertexShaderCode, fragShaderCode);

    initSquareBuffer();

    aPositionLocation = gl.getAttribLocation(shaderProgram, "aPosition");
    lightPosPosition = gl.getUniformLocation(shaderProgram, "lightPos");
    cameraPosPosition = gl.getUniformLocation(shaderProgram, "cameraPos");
    bounceLimitPosition = gl.getUniformLocation(shaderProgram, "bounceLimit");
    canvWidthPosition = gl.getUniformLocation(shaderProgram, "canvWidth");
    canvHeightPosition = gl.getUniformLocation(shaderProgram, "canvHeight");
    isShadowPosition = gl.getUniformLocation(shaderProgram, "isShadow");
    isReflectionPosition = gl.getUniformLocation(shaderProgram, "isReflection");

    //enable the attribute arrays
    gl.enableVertexAttribArray(aPositionLocation);
   
    var lightPosSlider = document.getElementById("lightPos");

    // Update the current slider value (each time you drag the slider handle)
    lightPosSlider.oninput = function() {
      lightPos[0] = this.value;
      drawScene();
    }

    var bounceLimitSlider = document.getElementById("bounceLimit");

    // Update the current slider value (each time you drag the slider handle)
    bounceLimitSlider.oninput = function() {
      bounceLimit = this.value;
      drawScene();
    }

    drawScene();
}