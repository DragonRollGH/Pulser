import Heart from "Heart";
import Cursor from "Cursor";
import PixelPos from "PixelPos";
import {hsv2rgb, btobit} from "../../utils/util"
// import * as mqtt from "../../utils/mqtt.min.4.1.10"
var mqtt = require("../../utils/mqtt.min.4.1.10.js")

const PixelRadius = 30;
const FingerRadius = 100;
const CanvasWidth = 750;
const CanvasHeight = 750;
const PixelColors = [0, 0.8, 0.9, 5, 35];
const flowPacketLen = 60;

const ctx = wx.createCanvasContext("Canvas");
const heart = new Heart(PixelPositions, PixelRadius, heartOptions);
const finger = new Finger(PixelPositions, FingerRadius);

const mqttOptions = {
  connectTimeout: 4000,  //超时时间
  clientId: 'wx_' + parseInt(Math.random() * 100 + 800, 10),
  // port: 443,
  username: "thingidp@ajdnaud|WebTest",
  password: "e6501f1b98c48378480bab53e5f3c8dc"
}

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

function onLoad() {
  wx.hideHomeButton();
  setInterval(animate, 17);
  animate();
}

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

function fingerTouchStart(event) {
  // event.preventDefault();
  finger.touchStart(event.changedTouches);
}

function fingerTouchMove(event) {
  finger.touchMove(event.changedTouches);
}

function fingerTouchEnd(event) {
  finger.touchMove(event.changedTouches);
}

function fingerTouchCancel(event) {
  finger.touchMove(event.changedTouches);
}

Page({
  data: {
    colorRes: "red"
  },
  changeHue: changeHue,
  onLoad: onLoad,
  preventDefault: ()=>{},
  fingerTouchStart: fingerTouchStart,
  fingerTouchMove: fingerTouchMove,
  fingerTouchEnd: fingerTouchEnd,
  fingerTouchCancel: fingerTouchCancel,
})
