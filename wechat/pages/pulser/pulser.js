import Pixel from "Pixel";
import Cursor from "Cursor";
import PixelPos from "PixelPos";
import {hsv2rgb} from "../../utils/Color"
// import * as mqtt from "../../utils/mqtt.min.4.1.10"
var mqtt = require("../../utils/mqtt.min.4.1.10.js")

const ctx = wx.createCanvasContext("Canvas");

const DPR = getApp().globalData.dpr;
const PixelLen = PixelPos.length;
const PixelRad = 30;
const TouchBox = 100;
const CanvasWidth = 750;
const CanvasHeight = 750;
const PixelColors = [0, 0.8, 0.9, 5, 35];

var pixels = [];
var cursors = [];

function findCursor(identifier, cursors) {
  for (let i in cursors) {
      if (cursors[i].identifier == identifier) {
          return i;
      }
  }
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
  wx.hideHomeButton();
  for (let i = 0; i < PixelLen; i++) {
    pixels.push(new Pixel(PixelPos[i][0], PixelPos[i][1], PixelRad));
  }
  // cursors.push(new Cursor());
  setInterval(animate, 17);
  animate();
  // const options = {
  //   connectTimeout: 4000,  //超时时间
  //   clientId: 'wx_' + parseInt(Math.random() * 100 + 800, 10),
  //   // port: 443,
  //   username: "thingidp@ajdnaud|WebTest",
  //   password: "e6501f1b98c48378480bab53e5f3c8dc"
  // }

  // var client = mqtt.connect('wxs://ajdnaud.iot.gz.baidubce.com/mqtt', options)
  // client.on('connect', (e) => {
  //   console.log('成功连接服务器!')
  //   this.setData({
  //     ok:"Connected"
  //   })
  // })
  // client.subscribe('Switch', {
  //   qos: 0
  // }, function (err) {
  //   if (!err) {
  //     console.log("订阅成功:Switch")
  //   }
  // })
  // client.on('message', function (topic, message, packet) {
  //   console.log(packet.payload.toString())
  // })
}

function preventDefault() {}

function changeHue(event) {
  let h = event.detail.value;
  PixelColors[0] = h;
  this.setData({
    colorRes: `rgb(${hsv2rgb(h, 1 ,1)})`
  })
}

// const page = {
//   pulserTouchStart: pulserTouchStart,
//   pulserTouchMove: pulserTouchMove,
//   pulserTouchEnd: pulserTouchEnd,
//   pulserTouchCancel: pulserTouchCancel,
//   changeHue: changeHue,
//   onLoad: onLoad
// }

Page({
  data: {
    colorRes: "red"
  },
  pulserTouchStart: pulserTouchStart,
  pulserTouchMove: pulserTouchMove,
  pulserTouchEnd: pulserTouchEnd,
  pulserTouchCancel: pulserTouchCancel,
  changeHue: changeHue,
  onLoad: onLoad
})