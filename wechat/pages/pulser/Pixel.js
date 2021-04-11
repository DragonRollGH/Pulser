import {hsv2rgb, hsl2rgba} from "../../utils/H2R"

const TWOPI = Math.PI * 2;

class Pixel {
  constructor(X, Y, R) {
    this.X = X;
    this.Y = Y;
    this.R = R;
    this.active = false;
  }
  run(H, S, L, A, B) {
    this.active = true;
    this.H = H;
    this.S = S;
    this.L = L;
    this.A = A;
    this.deltaL = this.L / B;
  }

  draw(ctx) {
    if (this.active) {
      // ctx.fillStyle = `rgba(${hsl2rgba(this.H, this.S, this.L)})`;
      let ga = 0;
      let gb = 1;
      if (this.L > 0.4) {
        ga = (this.L - 0.4)/0.6;
      }
      if (this.L < 0.6) {
        gb = this.L/0.6;
      }
      let grd = ctx.createCircularGradient(this.X, this.Y, this.R)
      grd.addColorStop(ga, `rgb(${hsv2rgb(this.H, this.S, 1)})`)
      grd.addColorStop(gb, `rgba(${hsv2rgb(this.H, this.S, 1)}, 0)`)
      ctx.fillStyle = grd;
      ctx.beginPath();
      ctx.arc(this.X, this.Y, this.R, 0, TWOPI);
      ctx.closePath();
      ctx.fill();
    }
  }

  updata() {
    if (this.active) {
      if (this.A) {
        this.A -= 1;
      } else {
        this.L -= this.deltaL;
        if (this.L <= 0) {
          this.active = false;
        }
      }
    }
  }
}

export default Pixel;