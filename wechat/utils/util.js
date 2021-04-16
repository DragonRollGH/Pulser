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
    let rgb = hsv2rgb(h, s, 1);
    rgb.push(l);
    return rgb;
}

export function btobit(bitString) {
  let asciis = [];
  for (let j = 0; j < Math.ceil(bitString.length / 8); ++j) {
    let bitString = "";
    for (let i = 0; i < 8; ++i) {
      let ij = 8 * j + i;
      if (ij < bitString.length) {
        bitString += bitString[ij];
      } else {
        bitString += '0';
      }
    }
    asciis.push(parseInt(bitString, 2));
  }
  return btoa(String.fromCharCode(...asciis));
}

export function bittob(baseString) {
  let asciiString = atob(baseString);
  let bitString = "";
  for (let i = 0; i < asciiString.length; ++i) {
    bitString += asciiString.charCodeAt(i).toString(2);
  }
  return bitString;
}