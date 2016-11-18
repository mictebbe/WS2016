var Vector2 = function (x, y) {
  this.x = x;
  this.y = y;
}

Vector2.prototype.dot = function (v) {
  return v.x*this.x + v.y*this.y;
}

Vector2.prototype.add = function (v) {
  return new Vector2(this.x+v.x, this.y+v.y);
}

Vector2.prototype.sub = function (v) {
  return new Vector2(this.x-v.x, this.y-v.y);
}

Vector2.prototype.scale = function (s) {
  return new Vector2(this.x*s, this.y*s);
}

Vector2.prototype.length = function () {
  return Math.sqrt(this.x*this.x+this.y*this.y);
}

Vector2.prototype.dist = function (v) {
  var distV = this.sub(v);
  return distV.length();
}

Vector2.prototype.normalize = function () {
  var len = this.length();
  var ret = new Vector2(this.x / len, this.y / len);
  return ret;
}





var Vector3 = function (x, y, z) {
  this.x = x;
  this.y = y;
  this.z = z;
}

Vector3.prototype.dot = function (v) {
  return v.x*this.x + v.y*this.y + v.z*this.z;
}

Vector3.prototype.add = function (v) {
  return new Vector3(this.x+v.x, this.y+v.y, this.z+v.z);
}

Vector3.prototype.sub = function (v) {
  return new Vector3(this.x-v.x, this.y-v.y, this.z-v.z);
}

Vector3.prototype.scale = function (s) {
  return new Vector3(this.x*s, this.y*s, this.z*s);
}

Vector3.prototype.length = function () {
  return Math.sqrt(this.x*this.x+this.y*this.y+this.z*this.z);
}

Vector3.prototype.dist = function (v) {
  var distV = this.sub(v);
  return distV.length();
}

Vector3.prototype.normalize = function () {
  var len = this.length();
  var ret = new Vector2(this.x / len, this.y / len, this.z / len);
  return ret;
}

Vector3.prototype.cross = function (v) {
  return new Vector3 (this.y*v.z - this.z*v.y,
                      this.z*v.x - this.x*v.z,
                      this.x*v.y - this.y*v.x);
}
