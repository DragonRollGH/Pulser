import {hsv2rgb, hsl2rgba} from "../../utils/util"

const TWOPI = Math.PI * 2;

class Pixel {
  constructor(x, y, r) {
    this.x = x;
    this.y = y;
    this.r = r;
    this.active = false;
  }

  draw(ctx) {
    ctx.fillStyle = "WhiteSmoke";
    ctx.beginPath();
    ctx.arc(this.x, this.y, this.r, 0, TWOPI);
    ctx.closePath();
    ctx.fill();
    if (this.active) {
      let ga = 0;
      let gb = 1;
      if (this.l > 0.4) {
        ga = (this.l - 0.4)/0.6;
      }
      if (this.l < 0.4) {
        gb = this.l/0.4;
      }
      let grd = ctx.createCircularGradient(this.x, this.y, this.r)
      grd.addColorStop(ga, `rgba(${hsv2rgb(this.h, this.s, 1)}, ${gb})`)
      grd.addColorStop(1, `rgba(${hsv2rgb(this.h, this.s, 1)}, 0)`)
      ctx.fillStyle = grd;
      // ctx.fillStyle = `rgba(${hsl2rgba(this.h, this.s, this.l)})`;
      ctx.beginPath();
      ctx.arc(this.x, this.y, this.r, 0, TWOPI);
      ctx.closePath();
      ctx.fill();
    }
  }
  
  run(colors) {
    this.active = true;
    this.h = colors[0];
    this.s = colors[1];
    this.l = colors[2];
    this.a = colors[3];
    this.deltaL = this.l / colors[4];
  }

  updata() {
    if (this.active) {
      if (this.a) {
        this.a -= 1;
      } else {
        this.l -= this.deltaL;
        if (this.l <= 0) {
          this.active = false;
        }
      }
    }
  }
}

export default Pixel;