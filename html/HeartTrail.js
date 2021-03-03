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

var f = 0;
var n = 0;

const mouse = {
    x: undefined,
    y: undefined,
    frontendIdOld: undefined,
    frontendIdNew: undefined,
    backendIdOld: undefined,
    backendIdNew: undefined
};
img.addEventListener('touchmove', function (event) {
    event.preventDefault();
    mouse.x = event.touches[0].clientX - CanvasCenterPageX;
    mouse.y = event.touches[0].clientY - CanvasCenterPageY;
})
function updateMouse(x, y) {
    var r = Math.sqrt(x ** 2 + y ** 2);
    var t = Math.atan2(y, x);
    if (TouchRegionMin < r && r < TouchRegionMax) {
        if (mouse.frontendIdNew) {
            mouse.frontendIdOld = mouse.frontendIdNew;
            mouse.backendIdOld = mouse.backendIdNew;
        }
        mouse.frontendIdNew = (Math.floor(t / WebPixelRad) + WebPixelLen) % WebPixelLen;
        mouse.backendIdNew = (Math.floor(t / DevPixelRad) + DevPixelLen) % DevPixelLen;
    }
    else {
        mouse.frontendIdOld = undefined;
        mouse.frontendIdNew = undefined;
        mouse.backendIdOld = undefined;
        mouse.backendIdNew = undefined;
    }
}

class Cursor {
    constructor() {
        // this.webPixelLen = webPixelLen;
        // this.webPixelRad = webPixelRad;
        // this.devPixelLen = devPixelLen;
        // this.devPixelRad = devPixelRad;
        this.x = undefined;
        this.y = undefined;
        this.webPixelIds = [undefined, undefined];
        this.devPixelIds = [undefined, undefined];
    }
    handleMouse(event) {
        this.x = event.x - CanvasCenterPageX;
        this.y = event.y - CanvasCenterPageY;
    }
    handleTouch(event) {
        event.preventDefault();
        this.x = event.touches[0].pageX - CanvasCenterPageX;
        this.y = event.touches[0].pageY - CanvasCenterPageY;
    }
    updata() {
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
    constructor(id, pixelLen, pixelRad) {
        this.id = id
        this.pixelLen = pixelLen;
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
    draw() {
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

function runCursoredPixel(pixelIds, pixels, pixelLen) {
    if (pixelIds[1] >= 0) {
        if (pixelIds[0] >= 0) {
            var diff = pixelIds[1] - pixelIds[0];
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
    cursor.updata();
    runCursoredPixel(cursor.webPixelIds, webPixels, WebPixelLen);
    // f = ++f % 3;
    // if (f == 0) {
    //     webPixels[n].run();
    //     n = ++n % WebPixelLen;
    // }
    for (let i = 0; i < WebPixelLen; i++) {
        webPixels[i].updata();
        webPixels[i].draw();
    }
    requestAnimationFrame(animate);
}

function init() {
    for (let i = 0; i < WebPixelLen; i++) {
        webPixels.push(new Pixel(i, WebPixelLen, WebPixelRad));
    }
    cursor = new Cursor();
    img.addEventListener('mousemove', function(event) {
        cursor.x = event.x - CanvasCenterPageX;
        cursor.y = event.y - CanvasCenterPageY;
    });
}

init();
animate();

