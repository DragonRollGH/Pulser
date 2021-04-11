import Pixel from "Pixel"
import Cursor from "Cursor"
import PixelPos from "PixelPos"

const ctx = wx.createCanvasContext("Canvas");

const DPR = getApp().globalData.dpr;
const PixelLen = PixelPos.length;
const PixelRad = 30;
const TouchBox = 100;
const CanvasWidth = 700;
const CanvasHeight = 700;
const PixelColors = [0, 1, 0.9, 10, 35];

var pixels = [];
var cursors = [];

function findCursor(identifier, cursors) {
  for (let i in cursors) {
      if (cursors[i].identifier == identifier) {
          return i;
      }
  }
}

function initCanvas(ctx) {
  ctx.scale(1 / DPR, 1 / DPR);
  // ctx.fillStyle = "rgba(255,0,0,0.5)";
  // ctx.fillStyle = `rgba(${hsl2rgba(0,1,0.5)})`;
  // ctx.beginPath();
  // ctx.arc(300, 300, PixelRad, 0, TWOPI);
  // ctx.closePath();
  // ctx.fill();
  // ctx.fillRect(0, 0, 300, 300);
  // let grd = ctx.createCircularGradient(50, 50, 30)
  // grd.addColorStop(0, 'red')
  // grd.addColorStop(1, 'white')
  // ctx.fillStyle = grd;
  // ctx.fillRect(10, 10, 150, 80)
  ctx.draw();
}

function animate() {
  for (let i in cursors) {
    cursors[i].updata()
    cursors[i].runPixels(pixels, PixelColors)
  }
  ctx.clearRect(0, 0, CanvasWidth, CanvasHeight);
  ctx.scale(1 / DPR, 1 / DPR);
  for (let i in pixels) {
    pixels[i].updata();
    pixels[i].draw(ctx);
  }
  ctx.draw();
}

function pulserTouchStart(event) {
  // event.preventDefault();
  for (let i in event.changedTouches) {
    var cursor = new Cursor(TouchBox);
    cursor.copy(event.changedTouches[i]);
    cursors.push(cursor);
  }
}

function pulserTouchMove(event) {
  for (let i in event.changedTouches) {
    var idx = findCursor(event.changedTouches[i].identifier, cursors);
    cursors[idx].copy(event.changedTouches[i]);
  }
}

function pulserTouchEnd(event) {
  for (let i in event.changedTouches) {
    var idx = findCursor(event.changedTouches[i].identifier, cursors);
    cursors.splice(idx, 1);
  }
}

function pulserTouchCancel(event) {
  for (let i in event.changedTouches) {
    var idx = findCursor(event.changedTouches[i].identifier, cursors);
    cursors.splice(idx, 1);
  }
}

function onLoad() {
  for (let i = 0; i < PixelLen; i++) {
    pixels.push(new Pixel(PixelPos[i][0], PixelPos[i][1], PixelRad));
    pixels[i].run(PixelColors);
  }
  // cursors.push(new Cursor());
  initCanvas(ctx);
  setInterval(animate, 17);
  animate();
}

Page({
  pulserTouchStart: pulserTouchStart,
  pulserTouchMove: pulserTouchMove,
  pulserTouchEnd: pulserTouchEnd,
  pulserTouchCancel: pulserTouchCancel,
  onLoad: onLoad
})