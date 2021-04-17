import Pixel from "Pixel";
import {btobit} from "../../utils/util"

const DPR = 1 / getApp().globalData.dpr;

class Heart {
  constructor(Positons, Radius, options) {
    this.pixels = [];
    for (let i = 0; i < Positons.length; i++) {
      this.pixels.push(new Pixel(Positons[i][0], Positons[i][1], Radius));
    }
    this.options = {
      canvasWidth: undefined,
      canvasHeight: undefined,
      ctx: undefined,
      fragmentation: 20,
      publishHandler: undefined,
    };
    this.setOptions(options);
    this.stream = "";
    this.streamFrame = 0;
  }

  draw() {
    this.options.ctx.clearRect(0, 0, this.options.canvasWidth, this.options.canvasHeight);
    this.options.ctx.scale(DPR, DPR);
    for (let i in this.pixels) {
      this.pixels[i].updata();
      this.pixels[i].draw(this.options.ctx);
    }
    this.options.ctx.draw();
  }

  publish() {
    let argN = "";
    let anyActive = 0;
    for (let i in this.pixels) {
      if (this.pixels[i].active) {
        anyActive = 1;
        argN += '1';
      } else {
        argN += '0';
      }
    }
    if (anyActive) {
      if (!this.streamFrame) {
        this.streamStart();
      }
      this.streamWriteN(argN);
    }
    if (!anyActive || this.streamFrame == this.options.fragmentation) {
      if (this.streamFrame) {
        this.streamEnd();
      }
    }
  }

  setOptions(options) {
    for (k in options) {
      if (typeof this.options[k] !== 'undefined') {
        this.options[k] = options[k]
      }
    }
  }

  streamStart() {
    this.streamFrame = 1;
    this.stream = ":H"
  }

  streamWriteN(argN) {
    let baseN = btobit(argN);
    this.stream += `&N${baseN};`;
    ++this.streamFrame;
  }

  streamEnd() {
    this.streamFrame = 0;
    // this.options.publishHandler(this.stream);
    console.log(this.stream);
    this.stream = "";
  }

  updata() {
    this.draw();
    this.publish();
  }

}