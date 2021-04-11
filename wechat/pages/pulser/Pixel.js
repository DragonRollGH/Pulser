import {hsv2rgb, hsl2rgba} from "../../utils/H2R"

const TWOPI = Math.PI * 2;

class Pixel {
  constructor(x, y, r) {
    this.x = x;
    this.y = y;
    this.r = r;
    this.active = false;
  }
  run(h, s, l, a, b) {
    this.active = true;
    this.h = h;
    this.s = s;
    this.l = l;
    this.a = a;
    this.deltaL = this.l / b;
  }

  draw(ctx) {
    if (this.active) {
      // ctx.fillStyle = `rgba(${hsl2rgba(this.h, this.s, this.l)})`;
      let ga = 0;
      let gb = 1;
      if (this.l > 0.4) {
        ga = (this.l - 0.4)/0.6;
      }
      if (this.l < 0.6) {
        gb = this.l/0.6;
      }
      let grd = ctx.createCircularGradient(this.x, this.y, this.r)
      grd.addColorStop(ga, `rgb(${hsv2rgb(this.h, this.s, 1)})`)
      grd.addColorStop(gb, `rgba(${hsv2rgb(this.h, this.s, 1)}, 0)`)
      ctx.fillStyle = grd;
      ctx.beginPath();
      ctx.arc(this.x, this.y, this.r, 0, TWOPI);
      ctx.closePath();
      ctx.fill();
    }
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