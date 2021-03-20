var canvas = document.getElementById("Heart");
var img = document.getElementById("Img");
var ctx = canvas.getContext("2d");

const CanvasCenterX = canvas.width / 2;
const CanvasCenterPageX = canvas.getBoundingClientRect().left + canvas.width / 2;
const CanvasCenterY = canvas.height / 2;
const CanvasCenterPageY = canvas.getBoundingClientRect().top + canvas.height / 2;

const TWOPI = Math.PI * 2;

const WebPixelLen = 200;          //页面的像素数
const WebPixelRad = TWOPI / WebPixelLen;
const DevPixelLen = 16;           //设备的像素数
const DevPixelRad = TWOPI / DevPixelLen;
const TouchRegionMin = 30;
const TouchRegionMax = 150;
const StreamFrame = 60;

var webPixels = [];
var devPixels = [];
var cursors = [];
var streams = [];
var streamTick = 0;
var streamValid = false;

class Cursor {
    constructor() {
        this.x = undefined;
        this.y = undefined;
        this.identifier = undefined;
        this.webPixelIds = [undefined, undefined];      //index 0 is the new id, 1 is the old id
        this.devPixelIds = [undefined, undefined];      //index 0 is the new id, 1 is the old id
    }
    copy(cursor) {
        this.x = cursor.pageX - CanvasCenterPageX;
        this.y = cursor.pageY - CanvasCenterPageX;
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

function handleMouse(event) {
    function handlemove(event) {
        cursors[0].copy(event);
    }
    event.preventDefault();
    addEventListener('mousemove', handlemove);
    addEventListener('mouseup', function () {
        removeEventListener('mousemove', handlemove);
        cursors[0].clear();
    });
    handlemove(event);
}

function handleTouchStart(event) {
    event.preventDefault();
    for (let i in event.changedTouches) {
        var cursor = new Cursor();
        cursor.copy(event.changedTouches[i]);
        cursors.push(cursor);
    }
}

function handleTouchMove(event) {
    for (let i in event.changedTouches) {
        var idx = findCursor(event.changedTouches[i].identifier);
        cursors[idx].copy(event.changedTouches[i]);
    }
}

function handleTouchEnd(event) {
    for (let i in event.changedTouches) {
        var idx = findCursor(event.changedTouches[i].identifier);
        cursors.splice(idx, 1);
    }
}

function findCursor(identifier) {
    for (let i in cursors) {
        if (cursors[i].identifier == identifier) {
            return i;
        }
    }
}

function runCursoredPixel(pixelIds, pixels) {
    if (pixelIds[1] >= 0) {
        if (pixelIds[0] >= 0) {
            var diff = pixelIds[1] - pixelIds[0];
            var pixelLen = pixels.length;
            diff = (diff + pixelLen) % pixelLen;
            var a = 2;
            var b = pixelLen / 4;
            if (diff < a || diff > pixelLen - a || (b < diff && diff < pixelLen - b)) {
                pixels[pixelIds[1]].run();
            } else {
                var s = diff < pixelLen / 2 ? 1 : -1;
                for (let i = pixelIds[0]; i != pixelIds[1];) {
                    i = (i + s + pixelLen) % pixelLen;
                    pixels[i].run();
                }
            }
        } else {
            pixels[pixelIds[1]].run();
        }
    }
}

function appendStream(devPixels) {
    if (streamTick == 0) {
        if (streamValid) {
            s = zip(streams.join(""))
            console.log(s);
            console.log("***");
            // console.log(s.length)
            console.log(streams.join(""));
            console.log("********************");
        }
        streamValid = false;
        for (let i in streams) {
            streams[i] = "";
        }
    }
    for (let i in devPixels) {
        if (devPixels[i].active) {
            streamValid = true;
            rgb = devPixels[i].getDevColor();
            for (let i in rgb) { streams[streamTick] += String.fromCharCode(rgb[i]) }
        } else {
            streams[streamTick] += "\x00\x00\x00";
        }
    }
    streams[streamTick] = btoa(streams[streamTick]);
}

function PUBLISH(msg) {
    msg;
}

function hsv2rgb(h, s, v) {
    var r, g, b;
    var i = Math.round((h / 60) % 6);
    var f = h / 60 - i;
    var p = v * (1 - s);
    var q = v * (1 - f * s);
    var t = v * (1 - (1 - f) * s);
    switch (i) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
        default: break;
    }
    return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
}

function zip(s) {
    zipStr = "";
    p = "";     //Previous String
    count = 0;
    for (let i = 0; i < s.length; i += 4) {
        c = s.substr(i, 4);     //Current String
        if (count == 0xff) {
            count = 0;
            p = "";
            zipStr += "*ff";
        }
        if (c != p) {
            if (count) {
                zipStr += "*" + count.toString(16).padStart(2, "0");
                count = 0;
            }
            p = c;
            zipStr += c;
        } else {
            ++count;
        }
    }
    if (count) { zipStr += "*" + count.toString(16).padStart(2, "0"); }
    return zipStr;
}

function animate() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.fillStyle = "white";
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    for (let i in cursors) {
        cursors[i].updata()
        runCursoredPixel(cursors[i].webPixelIds, webPixels);
        runCursoredPixel(cursors[i].devPixelIds, devPixels);
    }
    for (let i in webPixels) {
        webPixels[i].updata();
        webPixels[i].draw(ctx);
    }
    for (let i in devPixels) {
        devPixels[i].updata();
    }
    appendStream(devPixels);
    streamTick = (++streamTick) % StreamFrame;
    requestAnimationFrame(animate);
}

onload = function () {
    for (let i = 0; i < WebPixelLen; i++) {
        webPixels.push(new Pixel(i, WebPixelRad));
    }
    for (let i = 0; i < DevPixelLen; i++) {
        devPixels.push(new Pixel(i, DevPixelRad));
    }
    for (let i = 0; i < StreamFrame; i++) {
        streams.push("");
    }
    cursors.push(new Cursor());
    img.addEventListener('mousedown', handleMouse);
    img.addEventListener('touchstart', handleTouchStart);
    img.addEventListener('touchmove', handleTouchMove);
    img.addEventListener('touchend', handleTouchEnd);
    animate();
}

