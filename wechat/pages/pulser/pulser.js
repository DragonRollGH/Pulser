const PixelPositions = [
  [0,10.5966],
  [-4.2579,14.7652],
  [-9.7407,16.8550],
  [-15.4258,15.4034],
  [-19.2355,10.9408],
  [-19.7774,5.0983],
  [-16.8597,0.0047],
  [-12.6448,-4.2102],
  [-8.4298,-8.4251],
  [-4.2149,-12.6401],
  [0,-16.8550],
  [4.2149,-12.6401],
  [8.4298,-8.4251],
  [12.6448,-4.2102],
  [16.8597,0.0047],
  [19.7774,5.0983],
  [19.2355,10.9408],
  [15.4258,15.4034],
  [9.7407,16.8550],
  [4.2579,14.7652]
]

const ctx = wx.createCanvasContext("myCanvas")

class Cursor {
  constructor() {
      this.x = undefined;
      this.y = undefined;
      this.identifier = undefined;
      this.webPixelIds = [undefined, undefined];      //index 0 is the new id, 1 is the old id
      this.devPixelIds = [undefined, undefined];      //index 0 is the new id, 1 is the old id
  }
  copy(cursor) {
      this.x = cursor.pageX;// - CanvasCenterPageX;
      this.y = cursor.pageY;// - CanvasCenterPageX;
      this.identifier = cursor.identifier;
  }
  clear() {
      this.x = undefined;
      this.y = undefined;
      this.identifier = undefined;
  }
  updata() {
      if (this.x == undefined || this.y == undefined) {
          this.webPixelIds = [undefined, undefined];
          this.devPixelIds = [undefined, undefined];
          return
      }
      this.r = Math.sqrt(this.x ** 2 + this.y ** 2);
      this.t = Math.atan2(this.y, this.x);
      if (TouchRegionMin < this.r && this.r < TouchRegionMax) {
          if (this.webPixelIds[1] >= 0) {
              this.webPixelIds[0] = this.webPixelIds[1];
              this.devPixelIds[0] = this.devPixelIds[1];
          }
          this.webPixelIds[1] = (Math.floor(this.t / WebPixelRad) + WebPixelLen) % WebPixelLen;
          this.devPixelIds[1] = (Math.floor(this.t / DevPixelRad) + DevPixelLen) % DevPixelLen;
      } else {
          this.webPixelIds = [undefined, undefined];
          this.devPixelIds = [undefined, undefined];
      }
  }
}

class Pixel {
  constructor(id, pixelRad) {
      this.id = id
      this.pixelRad = pixelRad;
      this.startRad = (this.id - 0.5) * this.pixelRad;
      this.endRad = this.startRad + this.pixelRad;
      this.active = false;
  }
  run() {
      this.active = true;
      this.liveTime = 5;
      this.deadTime = 35;
      this.brightness = 0.8;
      this.hue = 0;
      this.alpha = this.brightness;
  }

  getWebColor() {
      this.webColor = "rgb(" + hsv2rgb(this.hue, this.alpha, 1).join() + ")";
      return this.webColor;
  }
  getDevColor() {
      this.devColor = hsv2rgb(this.hue, 1, this.alpha);
      return this.devColor
  }
  draw(ctx) {
      if (this.active) {
          ctx.beginPath();
          ctx.arc(CanvasCenterX, CanvasCenterY, TouchRegionMax, this.startRad, this.endRad);
          ctx.lineTo(CanvasCenterX, CanvasCenterY);
          ctx.closePath();
          ctx.fillStyle = this.getWebColor();
          ctx.fill();
      }
  }
  updata() {
      if (this.active) {
          if (this.liveTime) {
              this.liveTime -= 1;
          } else if (this.alpha > 0) {
              this.alpha -= this.brightness / this.deadTime;
          } else {
              this.active = false;
          }
      }
  }
}

// const qry = wx.createSelectorQuery()


Page({
  cursors: [],

  findCursor: function (identifier) {
    for (let i in this.cursors) {
      if (this.cursors[i].identifier == identifier) {
        return i;
      }
    }
  },

  handleTouchStart: function (event) {
    for (let i in event.changedTouches) {
      var cursor = new Cursor();
      cursor.copy(event.changedTouches[i]);
      this.cursors.push(cursor);
    }
  },

  handleTouchMove: function (event) {
    for (let i in event.changedTouches) {
      var idx = this.findCursor(event.changedTouches[i].identifier);
      this.cursors[idx].copy(event.changedTouches[i]);
    }
    console.log(this.cursors[0].x, this.cursors[0].y);
  },

  handleTouchEnd: function (event) {
    for (let i in event.changedTouches) {
      var idx = this.findCursor(event.changedTouches[i].identifier);
      this.cursors.splice(idx, 1);
    }
  },

  handleTouchCancel: function (event) {

  },
  draw: function (event) {
    var pr = 750 / wx.getSystemInfoSync().windowWidth;
    ctx.scale(1/pr, 1/pr)
    ctx.setFillStyle("red")
    for (let i in PixelPositions) {
      ctx.beginPath()
      ctx.arc(300+PixelPositions[i][0]*12, 300-PixelPositions[i][1]*12, 20, 0, 2*Math.PI)
      ctx.closePath()
      ctx.fill()
    }
    // ctx.fillRect(0,0,300,300)
    ctx.draw()
  },
  onLoad: function (event) {
    this.draw()
  }
})