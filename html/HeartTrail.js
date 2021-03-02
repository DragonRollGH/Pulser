const TWOPI = Math.PI * 2;

const frontendPoints = 60;          //页面的像素数
const frontendScale = TWOPI / frontendPoints;
const backendPoints = 24;           //设备的像素数
const backendScale = TWOPI / backendPoints;

var canvas = document.getElementById("Heart");
var img = document.getElementById("Img");
var ctx = canvas.getContext("2d");

const canvasCenterX = canvas.width / 2;
const pageCenterX = canvas.offsetLeft + canvasCenterX;
const canvasCenterY = canvas.height / 2;
const pageCenterY = canvas.offsetHeight + canvasCenterY;
const heartRadiusMin = 30;
const heartRadiusMax = 150;

var frontendNodes = [];
var f = 0;
var n = 0;
？？？
const mouse = {
    x: undefined,
    y: undefined,
    frontendIdOld: undefined,
    frontendIdNew: undefined,
    backendIdOld: undefined,
    backendIdNew: undefined
};
img.addEventListener('mousemove', function (event) {
    mouse.x = event.x - canvasCenterX;
    mouse.y = event.y - canvasCenterY;
})
img.addEventListener('touchmove', function (event) {
    event.preventDefault();
    mouse.x = event.touches[0].clientX - canvasCenterX;
    mouse.y = event.touches[0].clientY - canvasCenterY;
})
function updateMouse(x, y) {
    var r = Math.sqrt(x ** 2 + y ** 2);
    var t = Math.atan2(y, x);
    if (heartRadiusMin < r && r < heartRadiusMax) {
        if (mouse.frontendIdNew) {
            mouse.frontendIdOld = mouse.frontendIdNew;
            mouse.backendIdOld = mouse.backendIdNew;
        }
        mouse.frontendIdNew = (Math.floor(t / frontendScale) + frontendPoints) % frontendPoints;
        mouse.backendIdNew = (Math.floor(t / backendScale) + backendPoints) % backendPoints;
    }
    else {
        mouse.frontendIdOld = undefined;
        mouse.frontendIdNew = undefined;
        mouse.backendIdOld = undefined;
        mouse.backendIdNew = undefined;
    }
}

class frontendNode {
    constructor(id) {
        this.id = id
        this.startRad = this.id * frontendScale;
        this.endRad = this.startRad + frontendScale;
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
            ctx.arc(canvasCenterX, canvasCenterY, heartRadiusMax, this.startRad, this.endRad);
            ctx.lineTo(canvasCenterX, canvasCenterY);
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

function runFrontendNodes() {
    if (mouse.frontendIdNew >= 0) {
        if (mouse.frontendIdOld >= 0) {
            var diff = mouse.frontendIdNew - mouse.frontendIdOld;
            diff = (diff + frontendPoints) % frontendPoints;
            var a = 2;
            var b = frontendPoints / 4;
            if (diff < a || diff > frontendPoints - a || (b < diff && diff < frontendPoints - b)) {
                frontendNodes[mouse.frontendIdNew].run();
            }
            else {
                var s = diff < frontendPoints / 2 ? 1 : -1;
                for (let i = mouse.frontendIdOld; i != mouse.frontendIdNew;) {
                    i = (i + s + frontendPoints) % frontendPoints;
                    frontendNodes[i].run();
                }
            }
        }
        else {
            frontendNodes[mouse.frontendIdNew].run();
        }
    }
}


function animate() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    updateMouse(mouse.x, mouse.y);
    runFrontendNodes();
    // f = ++f % 3;
    // if (f == 0) {
    //     frontendNodes[n].run();
    //     n = ++n % frontendPoints;
    // }
    for (let i = 0; i < frontendPoints; i++) {
        frontendNodes[i].updata();
        frontendNodes[i].draw();
    }
    requestAnimationFrame(animate);
}

function init() {
    for (let i = 0; i < frontendPoints; i++) {
        frontendNodes.push(new frontendNode(i));
    }
}

init();
animate();
