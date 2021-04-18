import Finger from "Finger";
import Heart from "Heart";
import PixelPositions from "PixelPositions";
import {hsv2rgb} from "../../utils/util";
import * as MQTT from "../../utils/mqtt.min.4.1.10";

const ctx = wx.createCanvasContext("heart");

const CanvasWidth = 750;
const CanvasHeight = 750;
const FingerRadius = 100;
const HeartFragmentation = 30;
const PixelColors = [0, 0.8, 0.9, 5, 35];
const PixelRadius = 30;

const MqttUrl = "wxs://ajdnaud.iot.gz.baidubce.com/mqtt";
const mqttOptions = {
  connectTimeout: 5000,  //超时时间
  clientId: 'wx_' + new Date().getMilliseconds(),
  username: "thingidp@ajdnaud|PB_JS_1",
  password: "31926a99217eb5796fdd4794d684b0dc"
};
var mqtt = null;

const heartOptions = {
  canvasWidth: CanvasWidth,
  canvasHeight: CanvasHeight,
  ctx: ctx,
  fragmentation: HeartFragmentation,
};
const heart = new Heart(PixelPositions, PixelRadius, heartOptions);

const finger = new Finger(PixelPositions, FingerRadius);

function animate() {
  finger.update();
  heart.update(finger.cursoredIds, PixelColors);
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

function mqttConnect() {
  mqttOptions.clientId = 'wx_' + new Date().getMilliseconds();
  mqtt = MQTT.connect(MqttUrl, mqttOptions);
  heart.setOptions({mqtt: mqtt});
  mqttSubscribe();
}

function mqttSubscribe() {
  mqtt.on('connect', (e) => {console.log('成功连接服务器!');})
  mqtt.subscribe('PB/U/R', (err) => {if (!err) {console.log("订阅成功: PB/U/R");}})
  mqtt.on('message', function (topic, message, packet) {console.log(packet.payload.toString());})
}

function onHide() {
  mqtt.end();
  console.log('mqtt.end();');
}

function onLoad() {
  wx.hideHomeButton();
  setInterval(animate, 17);
  animate();
}


function onPullDownRefresh() {
  onHide();
  onShow();
}

function onShow() {
  mqttConnect();
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
  onHide: onHide,
  onLoad: onLoad,
  onPullDownRefresh: onPullDownRefresh,
  onShow: onShow,
  preventDefault: () => {},
});
