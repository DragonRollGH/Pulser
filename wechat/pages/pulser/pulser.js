import Finger from "Finger";
import Heart from "Heart";
import PixelPositions from "PixelPositions";
import {hsv2rgb, formatTime} from "../../utils/util";
import * as MQTT from "../../utils/mqtt.min.4.1.10";

var ctx = wx.createCanvasContext("heart");
const CanvasWidth = 750;
const CanvasHeight = 750;
var page;

const FingerRadius = 100;
var finger = new Finger(PixelPositions, FingerRadius);

const HeartFragmentation = 30;
const PixelRadius = 30;
var pixelColors = {
  H: 0,
  S: 255,
  L: 5,
  A: 5,
  B: 35
};
var heartOptions = {
  canvasWidth: CanvasWidth,
  canvasHeight: CanvasHeight,
  ctx: ctx,
  fragmentation: HeartFragmentation,
};
var heart = new Heart(PixelPositions, PixelRadius, heartOptions);

const MqttUrl = "wxs://ajdnaud.iot.gz.baidubce.com/mqtt";
var mqttOptions = {
  connectTimeout: 3000, //超时时间
  clientId: 'wx_' + new Date().getMilliseconds(),
  username: "thingidp@ajdnaud|PB_JS_1",
  password: "31926a99217eb5796fdd4794d684b0dc"
};
var mqtt = null;
var msgs = [""];
var pub = 0;
var pubButton = [
  "pubButtonR",
  "pubButtonM",
  "pubButtonL",
  "pubButtonM",
];
var pubTopic = [
  "PB/D/R",
  "PB/D/MR",
  "PB/D/M",
  "PB/D/MR",
];
var onlineL = false;
var onlineR = false;


function animate() {
  finger.update();
  heart.update(finger.cursoredIds, pixelColors);
  hueRainbow();
}

function cmdSend(event) {
  let cmd = event.detail.value.cmd;
  mqtt.publish(pubTopic[pub], cmd);
  printMsg(cmd);
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
  pixelColors.H = h;
  page.setData({
    hueBlock: `rgb(${hsv2rgb(Math.round(h * 359 / 255), 1 ,1)})`
  })
}

function hueChanging(event) {
  let h = event.detail.value;
  pixelColors.H = h;
  page.setData({
    hueBlock: `rgb(${hsv2rgb(Math.round(h * 359 / 255), 1 ,1)})`
  })
}

function huePress() {
  page.data.huePressed = !page.data.huePressed;
  wx.vibrateShort({});
}

function hueRainbow() {
  if (page.data.huePressed) {
    pixelColors.H = (pixelColors.H + 2) % 255;
    page.setData({
      hue: pixelColors.H,
      hueBlock: `rgb(${hsv2rgb(Math.round(pixelColors.H * 359 / 255), 1 ,1)})`,
    })
  }
}

function lightnessChange(event) {
  let l = event.detail.value;
  pixelColors.L = l;
}

function mqttConnect() {
  mqttOptions.clientId = 'wx_' + new Date().getMilliseconds();
  mqtt = MQTT.connect(MqttUrl, mqttOptions);
  heart.setOptions({
    mqtt: mqtt,
    pubTopic: pubTopic[pub]
  });
  mqttSubscribe();
}

function mqttSubscribe() {
  mqtt.on('connect', (e) => {
    console.log('成功连接服务器!');
    mqtt.publish(pubTopic[1], ':A');
  })
  mqtt.subscribe('PB/U/M', (err) => {
    if (!err) {
      console.log("订阅成功: PB/U/M");
    }
  })
  mqtt.subscribe('PB/U/R', (err) => {
    if (!err) {
      console.log("订阅成功: PB/U/R");
    }
  })
  mqtt.on('message', mqttMsg);
}

function mqttMsg(topic, message, packet) {
  console.log(topic.toString() + ': ' + message.toString());
  printMsg(topic.toString().substr(-1,1) + ': ' + message.toString());
  onlineCheck(topic);
}

function onlineCheck(topic) {
  if (!onlineL && topic == 'PB/U/M') {
    onlineSet('M');
  } else if (!onlineR && topic == 'PB/U/R') {
    onlineSet('R');
  }
}

function onlineSet(who) {
  if (who == 'MR') {
    page.setData({
      pubUserL: 'pubUserOn',
      pubUserR: 'pubUserOn',
    });
  } else if (who == "M") {
    page.setData({
      pubUserL: 'pubUserOn',
    });
  } else if (who == "R") {
    page.setData({
      pubUserR: 'pubUserOn',
    });
  }
}

function onHide() {
  mqtt.end();
  page.setData({
    pubUserL: 'pubUserOff',
    pubUserR: 'pubUserOff',
  });
}

function onLoad(event) {
  wx.hideHomeButton();
  page = this;
  pub = event.user*2;
  page.setData({
    pubButton: pubButton[pub],
  });
  setInterval(animate, 17);
  animate();
}

function onShow() {
  mqttConnect();
}

function printMsg(msg) {
  let newMsg = `${formatTime(new Date())} ${String(msg)}`;
  msgs.unshift(newMsg);
  if (msgs.length > 10) {
    msgs = msgs.slice(0, 10);
  }
  let setMsg = msgs.join("\n");
  page.setData({
    msg: setMsg
  })
}

function pubChange() {
  pub = ++pub % 4;
  page.setData({
      pubButton: pubButton[pub],
  })
  heart.setOptions({
    pubTopic: pubTopic[pub]
  });
}

Page({
  cmdSend: cmdSend,
  data: {
    hueBlock: 'red',
    huePressed: false,
    pubUserL: 'pubUserOff',
    pubUserR: 'pubUserOff',
    pubButton: pubButton[1]
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
  onShow: onShow,
  preventDefault: () => {},
  pubChange: pubChange,
});