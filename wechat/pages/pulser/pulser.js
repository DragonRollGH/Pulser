import Pixel from "Pixel";
import Cursor from "Cursor";
import PixelPos from "PixelPos";
import {hsv2rgb, btobit} from "../../utils/util"
// import * as mqtt from "../../utils/mqtt.min.4.1.10"
var mqtt = require("../../utils/mqtt.min.4.1.10.js")

const mqttOptions = {
  connectTimeout: 4000,  //超时时间
  clientId: 'wx_' + parseInt(Math.random() * 100 + 800, 10),
  // port: 443,
  username: "thingidp@ajdnaud|WebTest",
  password: "e6501f1b98c48378480bab53e5f3c8dc"
}

const ctx = wx.createCanvasContext("Canvas");

const DPR = getApp().globalData.dpr;
const PixelLen = PixelPos.length;
const PixelRad = 30;
const TouchBox = 100;
const CanvasWidth = 750;
const CanvasHeight = 750;
const PixelColors = [0, 0.8, 0.9, 5, 35];
const flowPacketLen = 60;

var pixels = [];
var cursors = [];
var flowFrame;
var flowStream;

function animate() {
  runPixels();
  drawPixels(ctx);
  publishPixels();
}

function changeHue(event) {
  let h = event.detail.value;
  PixelColors[0] = h;
  this.setData({
    colorRes: `rgb(${hsv2rgb(h, 1 ,1)})`
  })
}

function drawPixels(ctx) {
  ctx.clearRect(0, 0, CanvasWidth, CanvasHeight);
  ctx.scale(1 / DPR, 1 / DPR);
  for (let i in pixels) {
    pixels[i].updata();
    pixels[i].draw(ctx);
  }
  ctx.draw();
}

function findCursor(identifier, cursors) {
  for (let i in cursors) {
    if (cursors[i].identifier == identifier) {
      return i;
    }
  }
}

function flowStart() {
  flowFrame = 1;
  flowStream = ":H"
}

function flowWriteN(argN) {
  let baseN = btobit(argN);
  flowStream += `&N${baseN};`;
  ++flowFrame;
}

function flowEnd() {
  flowFrame = 0;
  // mqttPub(flowStream);
  console.log(flowStream);
}

function onLoad() {
  wx.hideHomeButton();
  for (let i = 0; i < PixelLen; i++) {
    pixels.push(new Pixel(PixelPos[i][0], PixelPos[i][1], PixelRad));
  }
  // cursors.push(new Cursor());
  setInterval(animate, 17);
  animate();

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

function publishPixels() {
  let argN = "";
  let anyActive = 0;
  for (let i in pixels) {
    if (pixels[i].active) {
      anyActive = 1;
      argN += '1';
    } else {
      argN += '0';
    }
  }
  if (anyActive) {
    if (!flowFrame) {
      flowStart();
    }
    flowWriteN(argN);
  }
  if (!anyActive || flowFrame == flowPacketLen) {
    if (flowFrame) {
      flowEnd();
    }
  }
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

function runPixels() {
  for (let i in cursors) {
    cursors[i].updata()
    cursors[i].runPixels(pixels, PixelColors)
  }
}

Page({
  data: {
    colorRes: "red"
  },
  changeHue: changeHue,
  onLoad: onLoad,
  preventDefault: ()=>{},
  pulserTouchStart: pulserTouchStart,
  pulserTouchMove: pulserTouchMove,
  pulserTouchEnd: pulserTouchEnd,
  pulserTouchCancel: pulserTouchCancel,
})
