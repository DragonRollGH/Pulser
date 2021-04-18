import Pixel from "Pixel";
import {btobit} from "../../utils/util";

const DPR = 1 / getApp().globalData.dpr;

class Heart {
  constructor(Positions, Radius, options) {
    this.pixels = [];
    for (let i = 0; i < Positions.length; i++) {
      this.pixels.push(new Pixel(Positions[i][0], Positions[i][1], Radius));
    }
    this.options = {
      canvasWidth: null,
      canvasHeight: null,
      ctx: null,
      fragmentation: 20,
      mqtt: null,
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
    for (let k in this.options) {
      if (typeof options[k] !== 'undefined') {
        this.options[k] = options[k];
      }
    }
  }

  streamStart() {
    this.streamFrame = 1;
    this.stream = ":H";
  }

  streamWriteN(argN) {
    let baseN = btobit(argN);
    this.stream += `&N${baseN};`;
    ++this.streamFrame;
  }

  streamEnd() {
    this.streamFrame = 0;
    if (this.options.mqtt) {
      this.options.mqtt.publish("PB/D/R", this.stream);
      console.log(this.stream);
    }
    this.stream = "";
  }

  update() {
    this.draw();
    this.publish();
  }
}

export default Heart;