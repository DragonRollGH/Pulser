const TWOPI = Math.PI * 2;

const WebPixelLen = 60;          //页面的像素数
const WebPixelRad = TWOPI / WebPixelLen;
const DevPixelLen = 24;           //设备的像素数
const DevPixelRad = TWOPI / DevPixelLen;

var canvas = document.getElementById("Heart");
var img = document.getElementById("Img");
var ctx = canvas.getContext("2d");

const CanvasCenterX = canvas.width / 2;
const CanvasCenterPageX = canvas.getBoundingClientRect().left + canvas.width / 2;
const CanvasCenterY = canvas.height / 2;
const CanvasCenterPageY = canvas.getBoundingClientRect().top + canvas.height / 2;
const TouchRegionMin = 30;
const TouchRegionMax = 150;

var webPixels = [];
var cursors = [];

class Cursor {
    constructor() {
        this.x = undefined;
        this.y = undefined;
        this.identifier = undefined;
        this.webPixelIds = [undefined, undefined];
        this.devPixelIds = [undefined, undefined];
    }
    copy(cursor) {
        this.x = cursor.pageX - CanvasCenterPageX;
        this.y = cursor.pageY - CanvasCenterPageX;
        this.identifier = cursor.identifier;
        // if (cursor.identifier != undefined) {
        // }
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
        }
        else {
            this.webPixelIds = [undefined, undefined];
            this.devPixelIds = [undefined, undefined];
        }
    }
}

class Pixel {
    constructor(id, pixelRad) {
        this.id = id
        this.pixelRad = pixelRad;
        this.startRad = this.id * this.pixelRad;
        this.endRad = this.startRad + this.pixelRad;
        this.active = false;
    }
    run() {
        this.active = true;
        this.liveTime = 20;
        this.deadTime = 30;
        this.hue = '255,0,0,';
        this.brightness = 0.8;
        this.alpha = this.brightness;
    }
    getColor() {
        this.color = 'rgba(' + this.hue + this.alpha + ')';
        return this.color;
    }
    draw(ctx) {
        if (this.active) {
            ctx.beginPath();
            ctx.arc(CanvasCenterX, CanvasCenterY, TouchRegionMax, this.startRad, this.endRad);
            ctx.lineTo(CanvasCenterX, CanvasCenterY);
            ctx.closePath();
            ctx.fillStyle = this.getColor();
            ctx.fill();
        }
    }
    updata() {
        if (this.active) {
            if (this.liveTime) {
                this.liveTime -= 1;
            }
            else if (this.alpha > 0) {
                this.alpha -= this.brightness / this.deadTime;
            }
            else {
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
            }
            else {
                var s = diff < pixelLen / 2 ? 1 : -1;
                for (let i = pixelIds[0]; i != pixelIds[1];) {
                    i = (i + s + pixelLen) % pixelLen;
                    pixels[i].run();
                }
            }
        }
        else {
            pixels[pixelIds[1]].run();
        }
    }
}

function animate() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    for (let i in cursors) {
        cursors[i].updata()
        runCursoredPixel(cursors[i].webPixelIds, webPixels);
    }
    for (let i in webPixels) {
        webPixels[i].updata();
        webPixels[i].draw(ctx);
    }
    requestAnimationFrame(animate);
}

window.onload = function () {
    for (let i = 0; i < WebPixelLen; i++) {
        webPixels.push(new Pixel(i, WebPixelRad));
    }
    cursors.push(new Cursor());
    img.addEventListener('mousedown', handleMouse);
    img.addEventListener('touchstart', handleTouchStart);
    img.addEventListener('touchmove', handleTouchMove);
    img.addEventListener('touchend', handleTouchEnd);
    animate();
}

