const LINE_DURATION = 2;
const LINE_WIDTH_START = 5;

$(document).ready(function() {
  enableDrawingCanvas();
  resizeCanvas(window.innerWidth, window.innerHeight);

  $(document).keypress(function(event) {
    if (event.key === "1") mode = 1;
    if (event.key === "2") mode = 2;

    if (event.key == "q") spread = 1;
    if (event.key == "w") spread = 2;

    if (event.key == "a") pathMode = 1;
    if (event.key == "s") pathMode = 2;

    if (event.key == "z") {
      if (lineWidthStart < 100) lineWidthStart++;
    }
    if (event.key == "x") {
      if (lineWidthStart > 1) lineWidthStart--;
    }

    if (event.key == "g") {
      if (canvas === undefined)
        enableDrawingCanvas();

      var poopx = new Array();
      var poopy = new Array();

      function spawn() {
        var gap = 10;
        var rows = 10;
        var cols = 3;

        var canvas = $('#myCanvas');
        var width = canvas.width();
        var height = canvas.height();
        var count = 0;

        for (var i = (width / 2) - (gap * rows); i < (width / 2) + (gap * rows); i = i + gap) {

          if (i % (gap * 2) === 0) {
            for (var j = (height / 2) - (gap * cols); j < (height / 2) + (gap * cols); j = j + gap) {

              poopx.push(i);
              poopy.push(j);

            }
          } else {
            for (var j = (height / 2) + (gap * cols) - gap; j > (height / 2) - (gap * cols) - gap; j = j - gap) {

              poopx.push(i);
              poopy.push(j);
            }
          }
        }
      }
      asd();

      function asd() {
        if (poopx.length <= 0) {
          spawn();
        }

        var x = poopx.pop();
        var y = poopy.pop();

        addPoint(x, y);

        setTimeout(function() {
          asd()
        }, 10);
      }
    }
  });
});

//////////////////////////
// Variable definitions //
//////////////////////////
var active = true;

var canvas;
var context;

var newWidth = 1000;
var newHeight = 800;

var mode = 1;
var pathMode = 1;
var spread = 2;

var lineColor = 'rgb(237, 184, 131)';
var lineDuration = LINE_DURATION;
var lineFadeLinger = 1;
var lineWidthStart = LINE_WIDTH_START;
var fadeDuration = 50;
var drawEveryFrame = 1; // Only adds a Point after these many 'mousemove' events

var clickCount = 0;
var frame = 0;

var flipNext = true;

var points = new Array();

///////////////////////
// Program functions //
///////////////////////

// Find canvas reference & enable listeners
function enableDrawingCanvas() {
  if (canvas === undefined) {
    canvas = document.getElementById('myCanvas');
    context = canvas.getContext('2d');
    enableListeners();
    init();
  }
}

// Initialize animation start
function init() {
  draw();
}

// Draw current state
function draw() {
  if (active) {
    animatePoints();
    window.requestAnimFrame(draw);
  }
}

// Update mouse positions
function animatePoints() {
  context.clearRect(0, 0, context.canvas.width, context.canvas.height);

  var duration = lineDuration * 1000 / 60;
  var point, lastPoint;

  if (pathMode === 2) {
    context.beginPath();
  }

  for (var i = 0; i < points.length; i++) {
    point = points[i];

    if (points[i - 1] !== undefined) {
      lastPoint = points[i - 1];
    } else {
      lastPoint = points[i];
    }

    point.lifetime += 1;

    if (point.lifetime > duration) {
      points.splice(i, 1);
      continue;
    }

    // Begin drawing stuff!
    var inc = (point.lifetime / duration); // 0 to 1 over lineDuration
    var dec = 1 - inc;

    var spreadRate;
    if (spread === 1) {
      spreadRate = lineWidthStart / (point.lifetime * 2);
    } // Lerp Decrease
    if (spread === 2) {
      spreadRate = lineWidthStart * (1 - inc);
    } // Linear Decrease

    var fadeRate = dec;

    //context.strokeStyle = lineColor;
    context.lineJoin = "round";
    context.lineWidth = spreadRate;
    context.strokeStyle = 'rgb(' + Math.floor(255) + ',' +
      Math.floor(200 - (255 * dec)) + ',' +
      Math.floor(200 - (255 * inc)) + ')';

    var distance = Point.distance(lastPoint, point);
    var midpoint = Point.midPoint(lastPoint, point);
    var angle = Point.angle(lastPoint, point);

    if (pathMode === 1) {
      context.beginPath();
    }

    if (mode === 1) {
      context.arc(midpoint.x, midpoint.y, distance / 2, angle, (angle + Math.PI), point.flip);
    }

    if (mode === 2) {
      context.moveTo(lastPoint.x, lastPoint.y);
      context.lineTo(point.x, point.y);
    }

    if (pathMode === 1) {
      context.stroke();
      context.closePath();
    }
  }

  if (pathMode === 2) {
    context.stroke();
    context.closePath();
  }

  //if (points.length > 0) { console.log(spreadRate + "|" + points.length + " points alive."); }
}

function addPoint(x, y) {
  flipNext = !flipNext;
  var point = new Point(x, y, 0, flipNext);
  points.push(point);
}

//////////////////////////////
// Less Important functions //
//////////////////////////////

// RequestAnimFrame definition
window.requestAnimFrame = (function(callback) {
  return window.requestAnimationFrame || window.webkitRequestAnimationFrame || window.mozRequestAnimationFrame || window.oRequestAnimationFrame || window.msRequestAnimationFrame ||
    function(callback) {
      window.setTimeout(callback, 1000 / 60);
    };
})();

// Update canvas dimensions based on input
function resizeCanvas(w, h) {
  if (context !== undefined) {
    context.canvas.width = w;
    context.canvas.height = h;

    newWidth = w;
    newHeight = h;
  }
}

// Listeners for mouse and touch events
function enableListeners() {

  //********* Mouse Listeners *********//
  $('#myCanvas').on('mousemove', function(e) {
    if (frame === drawEveryFrame) {
      addPoint(e.pageX - this.offsetLeft, e.pageY - this.offsetTop);
      frame = 0;
    }
    frame++;
  });

  $('#myCanvas').on('mouseover', function(e) {});
  $('#myCanvas').on('mouseleave', function(e) {});

  //********* Touch Listeners *********//
  $('#myCanvas').on('touchstart', function(e) {
    var touch = e.touches[0];
  });
  $('#myCanvas').on('touchmove', function(e) {
    var touch = e.touches[0];
  });
  $('#myCanvas').on('touchend', function(e) {});
}


// POINT CLASS
// Cartersian location of where mouse location
// was previously at.
// Used to draw arcs between Points.
var Point = class Point {

  // Define class constructor
  constructor(x, y, lifetime, flip) {
    this.x = x;
    this.y = y;
    this.lifetime = lifetime;
    this.flip = flip;
  }

  // Get the distance between a & b
  static distance(a, b) {
    const dx = a.x - b.x;
    const dy = a.y - b.y;

    return Math.sqrt(dx * dx + dy * dy);
  }

  // Get the mid point between a & b
  static midPoint(a, b) {
    const mx = a.x + (b.x - a.x) * 0.5;
    const my = a.y + (b.y - a.y) * 0.5;

    return new Point(mx, my);
  }

  // Get the angle between a & b
  static angle(a, b) {
    const dx = a.x - b.x;
    const dy = a.y - b.y;

    return Math.atan2(dy, dx);
  }

  // Simple getter for printing
  get pos() {
    return this.x + "," + this.y;
  }
}