import Finger from "Finger";
import Heart from "Heart";
import PixelPositions from "PixelPositions";
import {
  hsv2rgb
} from "../../utils/util";
import * as MQTT from "../../utils/mqtt.min.4.1.10";

const ctx = wx.createCanvasContext("heart");
var page;

const CanvasWidth = 750;
const CanvasHeight = 750;
const FingerRadius = 100;
const HeartFragmentation = 30;
const PixelColors = {
  H: 0,
  S: 255,
  L: 5,
  A: 5,
  B: 35
}
const PixelRadius = 30;

const MqttUrl = "wxs://ajdnaud.iot.gz.baidubce.com/mqtt";
const mqttOptions = {
  connectTimeout: 5000, //超时时间
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
  hueRainbow();
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

function hueChange(event) {
  let h = event.detail.value;
  PixelColors.H = h;
  this.setData({
    hueBlock: `rgb(${hsv2rgb(Math.round(h * 359 / 255), 1 ,1)})`
  })

  // h = ("0" + h.toString(16)).substr(-2);
  // mqtt.publish("PB/D/R", ":H&H" + h);
}

function hueChanging(event) {
  let h = event.detail.value;
  PixelColors.H = h;
  this.setData({
    hueBlock: `rgb(${hsv2rgb(Math.round(h * 359 / 255), 1 ,1)})`
  })
}

function huePress() {
  this.data.huePressed = !this.data.huePressed;
}

function hueRainbow() {
  if (page.data.huePressed) {
    PixelColors.H = (PixelColors.H + 2) % 255;
    page.setData({
      hue: PixelColors.H,
      hueBlock: `rgb(${hsv2rgb(Math.round(PixelColors.H * 359 / 255), 1 ,1)})`,
    })
  }
}

function lightnessChange(event) {
  let l = event.detail.value;
  PixelColors.L = l;
  // l = ("0" + l.toString(16)).substr(-2);
  // mqtt.publish("PB/D/R", ":H&L" + l);
}

function mqttConnect() {
  mqttOptions.clientId = 'wx_' + new Date().getMilliseconds();
  mqtt = MQTT.connect(MqttUrl, mqttOptions);
  heart.setOptions({
    mqtt: mqtt
  });
  mqttSubscribe();
}

function mqttSubscribe() {
  mqtt.on('connect', (e) => {
    console.log('成功连接服务器!');
  })
  mqtt.subscribe('PB/U/R', (err) => {
    if (!err) {
      console.log("订阅成功: PB/U/R");
    }
  })
  mqtt.on('message', function (topic, message, packet) {
    console.log(packet.payload.toString());
  })
}

function onHide() {
  mqtt.end();
  console.log('mqtt.end();');
}

function onLoad() {
  wx.hideHomeButton();
  page = this;
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
    hueBlock: "red",
    huePressed: false,
  },
  heartTouchStart: heartTouchStart,
  heartTouchMove: heartTouchMove,
  heartTouchEnd: heartTouchEnd,
  heartTouchCancel: heartTouchCancel,
  hueChange: hueChange,
  hueChanging: hueChanging,
  huePress: huePress,
  lightnessChange: lightnessChange,
  onHide: onHide,
  onLoad: onLoad,
  onPullDownRefresh: onPullDownRefresh,
  onShow: onShow,
  preventDefault: () => {},
});