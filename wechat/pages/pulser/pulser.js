import Finger from "Finger";
import Heart from "Heart";
import PixelPositions from "PixelPositions";
import {hsv2rgb} from "../../utils/util";
// import * as mqtt from "../../utils/mqtt.min.4.1.10";
var mqtt = require("../../utils/mqtt.min.4.1.10.js");

const ctx = wx.createCanvasContext("heart");

const CanvasWidth = 750;
const CanvasHeight = 750;
const FingerRadius = 100;
const HeartFragmentation = 30;
const PixelColors = [0, 0.8, 0.9, 5, 35];
const PixelRadius = 30;

const mqttOptions = {
  connectTimeout: 4000,  //超时时间
  clientId: 'wx_' + parseInt(Math.random() * 100 + 800, 10),
  // port: 443,
  username: "thingidp@ajdnaud|PB_JS_1",
  password: "31926a99217eb5796fdd4794d684b0dc"
};

const heartOptions = {
  canvasWidth: CanvasWidth,
  canvasHeight: CanvasHeight,
  ctx: ctx,
  fragmentation: HeartFragmentation,
  publishHandler: undefined,
};

const finger = new Finger(PixelPositions, FingerRadius);
const heart = new Heart(PixelPositions, PixelRadius, heartOptions);

function animate() {
  finger.update(heart.pixels, PixelColors);
  heart.update();
}

function changeHue(event) {
  let h = event.detail.value;
  PixelColors[0] = h;
  this.setData({
    colorRes: `rgb(${hsv2rgb(h, 1 ,1)})`
  })
}

function heartTouchStart(event) {
  // event.preventDefault();
  finger.touchStart(event.changedTouches);
}

function heartTouchMove(event) {
  finger.touchMove(event.changedTouches);
}

function heartTouchEnd(event) {
  finger.touchEnd(event.changedTouches);
}

function heartTouchCancel(event) {
  finger.touchEnd(event.changedTouches);
}

function onLoad() {
  wx.hideHomeButton();
  var client = mqtt.connect('wxs://ajdnaud.iot.gz.baidubce.com/mqtt', mqttOptions)
  client.on('connect', (e) => {
    console.log('成功连接服务器!')
    this.setData({
      ok:"Connected"
    })
  })
  client.subscribe('Switch', (err) => {
    if (!err) {
      console.log("订阅成功:Switch")
    }
  })
  client.on('message', function (topic, message, packet) {
    console.log(packet.payload.toString())
  })
  setInterval(animate, 17);
  animate();
}


Page({
  data: {
    colorRes: "red"
  },
  changeHue: changeHue,
  heartTouchStart: heartTouchStart,
  heartTouchMove: heartTouchMove,
  heartTouchEnd: heartTouchEnd,
  heartTouchCancel: heartTouchCancel,
  onLoad: onLoad,
  preventDefault: () => {},
});
