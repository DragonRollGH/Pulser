export function hsv2rgb(h, s, v) {
    var r, g, b;
    var i = h / 60;
    var c = v * s;
    var x = c * (1 - Math.abs(i % 2 - 1));
    var m = v - c;
    switch (Math.floor(i)) {
        case 0: r = c; g = x; b = 0; break;
        case 1: r = x; g = c; b = 0; break;
        case 2: r = 0; g = c; b = x; break;
        case 3: r = 0; g = x; b = c; break;
        case 4: r = x; g = 0; b = c; break;
        case 5: r = c; g = 0; b = x; break;
        default: r = 0; g = 0; b = 0;
    }
    r = Math.round(255 * (r + m));
    g = Math.round(255 * (g + m));
    b = Math.round(255 * (b + m));
    return [r, g, b];
}

export function hsl2rgba(h, s, l) {
    let rgba = hsv2rgb(h, s, 1);
    rgba.push(l);
    return rgba;
}

export function btobit(bitString) {
  let I = parseInt(bitString, 2);
  let a = [];
  while(I) {
    a.push(I % 256);
    I = Math.floor(I / 256);
  }
  // return btoa(String.fromCharCode(...a));
  return wx.arrayBufferToBase64(a);
}

export function bittob(baseString) {
  let a = atob(baseString);
  let b = "";
  for (let i = 0; i < a.length; ++i) {
    b += a.charCodeAt(i).toString(2);
  }
  return b;
}
