const DPR = getApp().globalData.dpr;

class Cursor {
  constructor() {
    this.x = undefined;
    this.y = undefined;
    this.identifier = undefined;
    this.pixelIds = [undefined, undefined]; //index 0 is the old id, 1 is the new id
  }
  copy(cursor) {
    if (cursor.pageX !== undefined) {
      this.x = cursor.pageX;
      this.y = cursor.pageY;
    } else {
      this.x = cursor.X * DPR;
      this.y = cursor.Y * DPR;
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

    for (let i in PixelLen) {
      dis = Math.sqrt((this.x - PixelPos[i][0]) ** 2 + (this.y - PixelPos[i][1]) ** 2);
      if (dis <= TouchBoxR) {
        this.pixelIds[1] = i;
        break;
      }
    }
  }
}

export default Cursor;