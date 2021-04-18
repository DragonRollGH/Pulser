import Pixel from "Pixel";
import { btobit } from "../../utils/util";

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
    this.cursoredIds = [];
    this.colors = {};
    this.oldColors = {};
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
    let anyCursored = 0;
    for (let i in this.pixels) {
      if (this.cursoredIds.includes(Number(i))) {
        anyCursored = 1;
        argN += "1";
      } else {
        argN += "0";
      }
    }

    if (anyCursored && !this.streamFrame) {
      this.streamStart();
    }
    if (this.streamFrame == this.options.fragmentation) {
      this.streamEnd();
    } else if (this.streamFrame) {
      this.streamWrite(argN);
    }
  }

  run() {
    for (let i in this.cursoredIds) {
      this.pixels[this.cursoredIds[i]].run(this.colors);
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

  streamWrite(argN) {
    for (let k in this.colors) {
      if (this.colors[k] !== this.oldColors[k]) {
        let c = ("0" + this.colors[k].toString(16)).substr(-2);
        this.stream += `&${k}${c}`;
        this.oldColors[k] = this.colors[k];
        // console.log(`&${k}${c}`);
      }
    }
    this.streamWriteN(argN);
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
    this.stream = ":H";
  }

  update(cursoredIds, pixelColors) {
    this.cursoredIds = cursoredIds;
    this.colors = pixelColors;
    this.run();
    this.draw();
    this.publish();
  }
}

export default Heart;