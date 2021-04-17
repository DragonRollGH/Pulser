const DPR = getApp().globalData.dpr;

class Cursor {
  constructor(pixelPositions, radius) {
    this.pixelPositions = pixelPositions;
    this.radius = radius;
    this.x = undefined;
    this.y = undefined;
    this.identifier = undefined;
    this.pixelIds = [undefined, undefined]; //index 0 is the old id, 1 is the new id
  }

  clear() { //for pc's mouse
    this.x = undefined;
    this.y = undefined;
    this.identifier = undefined;
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

  runPixels(pixels, pixelColors) {
    if (this.pixelIds[1] >= 0) {
      if (this.pixelIds[0] >= 0) {
        let diff = this.pixelIds[1] - this.pixelIds[0];
        let pixelLen = pixels.length;
        diff = (diff + pixelLen) % pixelLen;
        let a = 2;
        let b = pixelLen / 4;
        if (diff < a || diff > pixelLen - a || (b < diff && diff < pixelLen - b)) {
          pixels[this.pixelIds[1]].run(pixelColors);
        } else {
          let s = diff < pixelLen / 2 ? 1 : -1;
          for (let i = this.pixelIds[0]; i != this.pixelIds[1];) {
            i = (i + s + pixelLen) % pixelLen;
            pixels[i].run(pixelColors);
          }
        }
      } else {
        pixels[this.pixelIds[1]].run(pixelColors);
      }
    }
  }

  updata() {
    this.pixelIds[0] = this.pixelIds[1];
    this.pixelIds[1] = undefined;
    if (this.x == undefined || this.y == undefined) {
      this.pixelIds[0] = undefined;
      return
    }
    let distence = this.radius + 1;
    for (let i in this.pixelPositions) {
      let ndistence = Math.sqrt((this.x - this.pixelPositions[i][0]) ** 2 + (this.y - this.pixelPositions[i][1]) ** 2);
      if (ndistence < distence) {
        distence = ndistence;
        this.pixelIds[1] = Number(i);
      }
    }
    if (distence > this.radius) {
      this.pixelIds[1] = undefined;
    }
  }
}

export default Cursor;