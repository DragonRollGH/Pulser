export function hsv2rgb(h, s, v) {
    var r, g, b;
    var i = Math.round((h / 60) % 6);
    var f = h / 60 - i;
    var p = v * (1 - s);
    var q = v * (1 - f * s);
    var t = v * (1 - (1 - f) * s);
    switch (i) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
        default: break;
    }
    return [Math.round(r * 255), Math.round(g * 255), Math.round(b * 255)];
}

export function hsl2rgba(h, s, l) {
    let rgb = hsv2rgb(h, s, 1);
    rgb.push(l);
    return rgb;
}