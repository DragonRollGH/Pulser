import Cursor from "Cursor";

class Finger {
  constructor(pixelPositions, radius) {
    this.pixelPositions = pixelPositions;
    this.radius = radius;
    this.cursors = [];
    // this.cursors.push(new Cursor()) //for pc's mouse
    this.cursoredIds = [];
  }

  find(identifier) {
    for (let i in this.cursors) {
      if (this.cursors[i].identifier === identifier) {
        return i;
      }
    }
  }

  touchStart(changedTouches) {
    for (let i in changedTouches) {
      let cursor = new Cursor(this.pixelPositions, this.radius);
      cursor.copy(changedTouches[i]);
      this.cursors.push(cursor);
    }
  }

  touchMove(changedTouches) {
    for (let i in changedTouches) {
      let idx = this.find(changedTouches[i].identifier);
      this.cursors[idx].copy(changedTouches[i]);
    }
  }

  touchEnd(changedTouches) {
    for (let i in changedTouches) {
      var idx = this.find(changedTouches[i].identifier);
      this.cursors.splice(idx, 1);
    }
  }

  update() {
    this.cursoredIds = [];
    for (let i in this.cursors) {
      this.cursors[i].updata();
      this.cursoredIds.push(...this.cursors[i].extendCursoredIds);
    }
  }
}

export default Finger;