const DPR = getApp().globalData.dpr;

class Cursor {
  constructor(pixelPositions, radius) {
    this.pixelPositions = pixelPositions;
    this.radius = radius;
    this.x = null;
    this.y = null;
    this.identifier = null;
    this.cursoredIds = [undefined, undefined]; //index 0 is the old id, 1 is the new id
    this.extendCursoredIds = [];
  }

  clear() { //for pc's mouse
    this.x = null;
    this.y = null;
    this.identifier = null;
  }

  computePixelId() {
    this.cursoredIds[0] = this.cursoredIds[1];
    this.cursoredIds[1] = undefined;
    if (this.x == null || this.y == null) {
      this.cursoredIds[0] = undefined;
      return
    }
    let distence = this.radius + 1;
    for (let i in this.pixelPositions) {
      let ndistence = Math.sqrt((this.x - this.pixelPositions[i][0]) ** 2 + (this.y - this.pixelPositions[i][1]) ** 2);
      if (ndistence < distence) {
        distence = ndistence;
        this.cursoredIds[1] = Number(i);
      }
    }
    if (distence > this.radius) {
      this.cursoredIds[1] = undefined;
    }
  }

  copy(cursor) {
    if (cursor.pageX !== undefined) {
      this.x = cursor.pageX * DPR;
      this.y = cursor.pageY * DPR;
    } else {
      this.x = cursor.x * DPR;
      this.y = cursor.y * DPR;
    }
    this.identifier = cursor.identifier;
  }

  extendPixelId() {
    this.extendCursoredIds = [];
    if (this.cursoredIds[1] >= 0) {
      if (this.cursoredIds[0] >= 0) {
        let diff = this.cursoredIds[1] - this.cursoredIds[0];
        let pixelLen = this.pixelPositions.length;
        diff = (diff + pixelLen) % pixelLen;
        let a = 2;
        let b = pixelLen / 4;
        if (diff < a || diff > pixelLen - a || (b < diff && diff < pixelLen - b)) {
          this.extendCursoredIds.push(this.cursoredIds[1]);
          // pixels[this.cursoredIds[1]].run(pixelColors);
        } else {
          let s = diff < pixelLen / 2 ? 1 : -1;
          for (let i = this.cursoredIds[0]; i != this.cursoredIds[1];) {
            i = (i + s + pixelLen) % pixelLen;
            this.extendCursoredIds.push(i);
            // pixels[i].run(pixelColors);
          }
        }
      } else {
        this.extendCursoredIds.push(this.cursoredIds[1]);
        // pixels[this.cursoredIds[1]].run(pixelColors);
      }
    }
  }

  updata() {
    this.computePixelId();
    this.extendPixelId();
  }
}

export default Cursor;