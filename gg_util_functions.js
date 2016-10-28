function saw (phase) {
  return (phase % (Math.PI*2)) / 2 / Math.PI;
}

var constrain = function (v, min, max) {
  if (v<min) return min;
  if (v>max) return max;
  return v;
}

var blend = function (v0, v1, t) {
  return (1-t)*v0 + t*v1;
}

function setBrightness (b) {
  b = Math.round(b);
  context.fillStyle = "rgb("+b+", "+b+", "+b+")";
}

function rgbString (r, g, b) {
  r = Math.round(r);
  g = Math.round(g);
  b = Math.round(b);
  return "rgb("+r+", "+g+", "+b+")";
}
function setFillColor (r, g, b) {
  context.fillStyle = rgbString(r,g,b);
}

function setStrokeColor (r, g, b) {
  context.strokeStyle = rgbString(r,g,b);
}

function setColorHSL (h, s, l) {
  h = Math.round(h);
  s = s+"%";
  l = l+"%";

  context.fillStyle = "hsl("+h+","+s+","+l+")";
}

function blend3 (v0, v1, t) {
  return {
    x : blend(v0.x, v1.x, t),
    y : blend(v0.y, v1.y, t),
    z : blend(v0.z, v1.z, t)
  }
}


var SimpleNoise = function (N) {
  this.randomArray = [];
  for (var j=0; j<N; j++) {
    this.randomArray[j] = [];
    for (var i=0; i<N; i++) {
      this.randomArray[j][i] = Math.random();
    }
  }
}

// 1D noise function
SimpleNoise.prototype.getValue = function(x) {
  if (x<0) x*=-1;
  return this.randomArray[0][Math.round(x%this.randomArray.length)];
}


// 2D noise
SimpleNoise.prototype.getValue2 = function(x,y) {
  if (x<0) x*=-1;
  if (y<0) y*=-1;
  x = Math.round(x%this.randomArray.length);

  var index0 = Math.floor(y%this.randomArray.length);
  var index1 = (index0 + 1) % this.randomArray.length;
  var rest = y - index0;
  var rand0 = this.randomArray[x][index0];
  var rand1 = this.randomArray[x][index1];

  return blend (rand0, rand1, rest);
}
