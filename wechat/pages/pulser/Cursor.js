import PixelPos from "PixelPos"

const DPR = getApp().globalData.dpr;

class Cursor {
  constructor(touchBox) {
    this.touchBox = touchBox;
    this.x = undefined;
    this.y = undefined;
    this.identifier = undefined;
    this.pixelIds = [undefined, undefined]; //index 0 is the old id, 1 is the new id
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
  clear() {
    this.x = undefined;
    this.y = undefined;
    this.identifier = undefined;
  }
  updata() {
    this.pixelIds[0] = this.pixelIds[1];
    this.pixelIds[1] = undefined;
    if (this.x == undefined || this.y == undefined) {
      this.pixelIds[0] = undefined;
      return
    }
    let dist = this.touchBox + 1;
    for (let i in PixelPos) {
      let ndist = Math.sqrt((this.x - PixelPos[i][0]) ** 2 + (this.y - PixelPos[i][1]) ** 2);
      if (ndist < dist) {
        dist = ndist;
        this.pixelIds[1] = Number(i);
      }
    }
    if (dist > this.touchBox) {
      this.pixelIds[1] = undefined;
    }
  }
}

export default Cursor;