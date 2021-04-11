const PixelPos = [
  // [0,0],
  [300.0000, 172.8408],
  [248.9052, 122.8176],
  [183.1116, 97.7400],
  [114.8904, 115.1592],
  [69.1740, 168.7104],
  [62.6712, 238.8204],
  [97.6836, 299.9436],
  [148.2624, 350.5224],
  [198.8424, 401.1012],
  [249.4212, 451.6812],
  [300.0000, 502.2600],
  [350.5788, 451.6812],
  [401.1576, 401.1012],
  [451.7376, 350.5224],
  [502.3164, 299.9436],
  [537.3288, 238.8204],
  [530.8260, 168.7104],
  [485.1096, 115.1592],
  [416.8884, 97.7400],
  [351.0948, 122.8176]
]
const app = getApp();
const ctx = wx.createCanvasContext("Canvas");

const DPR = app.globalData.dpr;
const TWOPI = Math.PI * 2;
const PixelLen = 20;
const TouchBoxR = 35;
const DisplayR = 35;
const CanvasWidth = 600;
const CanvasHeight = 600;

var pixels = [];
var cursors = [];

function initCanvas(ctx) {
  ctx.scale(1 / DPR, 1 / DPR);
  ctx.fillStyle = "red";
  // ctx.beginPath();
  // ctx.arc(300, 300, DisplayR, 0, TWOPI);
  // ctx.closePath();
  // ctx.fill();
  // ctx.fillRect(0, 0, 300, 300);
  ctx.draw();
}

function animate() {
  // for (let i in cursors) {
  //   cursors[i].updata()
  //   // runCursoredPixel(cursors[i].pixelIds, webPixels);
  // }
  ctx.clearRect(0, 0, CanvasWidth, CanvasHeight);
  ctx.scale(1 / DPR, 1 / DPR);
  for (let i in pixels) {
    pixels[i].updata();
    pixels[i].draw(ctx);
  }
  ctx.draw();
}

class Cursor {
  constructor() {
    this.x = undefined;
    this.y = undefined;
    this.identifier = undefined;
    this.pixelIds = [undefined, undefined]; //index 0 is the old id, 1 is the new id
  }
  copy(cursor) {
    this.x = cursor.X * DPR;
    this.y = cursor.Y * DPR;
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

class Pixel {
  constructor(id) {
    this.id = id
    this.active = false;
  }
  run() {
    this.active = true;
    this.H = 0;
    this.S = 1;
    this.L = 0.5
    this.A = 5;
    this.deltaL = this.L / 35;
  }

  draw(ctx) {
    if (this.active) {
      // ctx.scale(1 / DPR, 1 / DPR);
      ctx.fillStyle = `red`;
      // ctx.fillStyle = `hsl(${this.H},${this.S},${this.L})`;
      // ctx.fillStyle(`hsl(${this.H},${this.S},${this.L})`);
      ctx.beginPath();
      ctx.arc(PixelPos[this.id][0], PixelPos[this.id][1], DisplayR, 0, TWOPI);
      ctx.closePath();
      ctx.fill();
    }
  }
  updata() {
    if (this.active) {
      if (this.A) {
        this.A -= 1;
      } else {
        this.L -= this.deltaL;
        if (this.L <= 0) {
          this.active = false;
        }
      }
    }
  }
}


Page({
  findCursor: function (identifier) {
    for (let i in this.cursors) {
      if (this.cursors[i].identifier == identifier) {
        return i;
      }
    }
  },

  pageTouchStart: function (event) {
    for (let i in event.changedTouches) {
      var cursor = new Cursor();
      cursor.copy(event.changedTouches[i]);
      this.cursors.push(cursor);
    }
  },

  pageTouchMove: function (event) {
    for (let i in event.changedTouches) {
      var idx = this.findCursor(event.changedTouches[i].identifier);
      this.cursors[idx].copy(event.changedTouches[i]);
    }
    // console.log(this.cursors[0].x, this.cursors[0].y);
    console.log(event)
  },

  pageTouchEnd: function (event) {
    for (let i in event.changedTouches) {
      var idx = this.findCursor(event.changedTouches[i].identifier);
      this.cursors.splice(idx, 1);
    }
  },

  pageTouchCancel: function (event) {

  },
  onLoad: function (event) {
    for (let i = 0; i < PixelLen; i++) {
      pixels.push(new Pixel(i));
      pixels[i].run();
    }
    // cursors.push(new Cursor());
    initCanvas(ctx);
    setInterval(animate, 1000);
    animate();
  }
})