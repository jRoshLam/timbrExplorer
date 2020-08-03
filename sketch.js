let trill1;
let trill2;
let belaLogo;

function preload() {
	belaLogo = loadImage('../images/logo_bar14.png');
	loadScript("/libraries/Trill/Trill.js");
}

function setup() {
	createCanvas(windowWidth, windowHeight);
	trill1 = new Trill('square');
	trill2 = new Trill('square');
	windowResized();
}

function draw() {
	background(240);

	// get touch size of Trill sensors
	var touch1Size = Bela.data.buffers[1][0];
	var touch2Size = Bela.data.buffers[3][0];
	
	// If touch size is large enough update touch positions
	// Since javascript functions are objects, their class members are persistent
	// if (touch1Size > 0.1) {
	// 	draw.touch1Position =  Bela.data.buffers[0];
	// }
	// if (touch2Size > 0.1) {
	// 	draw.touch2Position =  Bela.data.buffers[2];
	// }
	draw.touch1Position =  Bela.data.buffers[0];
	draw.touch2Position =  Bela.data.buffers[2];
	
	let c = color(0, 0, 0);
	fill(c);
	textSize(24);
	textAlign(CENTER);
	text('Brightness', trill1.xpos, height /2 + trill1.width / 1.75);
	text('Spectrum', width * 0.035, height / 2, width * 0.06, 32);
	
	text('Envelope', trill2.xpos, height /2 + trill1.width / 1.75);
	text('Articulation', width * 0.54, height / 2, width * 0.06, 32);

	// Only update touches if they have been initialized. Draw trills
	if ( typeof draw.touch1Position != 'undefined' ) {
		trill1.updateTouch(0, draw.touch1Position, 0.2);
	}
	trill1.draw();
	
	if ( typeof draw.touch2Position != 'undefined' ) {
		trill2.updateTouch(0, draw.touch2Position, 0.2);
	}
	trill2.draw();
	
	// Add BELA logo
	image(belaLogo,width-170, height-70,120,50);
}

function windowResized() {
	resizeCanvas(windowWidth, windowHeight);
	var trWidth = width * 0.35;
	trill1.width = width * 0.35;
	trill1.resize(trill1.width);
	var tr1X = width * 0.07 + trWidth/2;
	trill1.xpos = tr1X;
	trill1.position = [tr1X, height / 2];
	
	trill2.resize(trWidth);
	var tr2X = width * 0.93 - trWidth/2;
	trill2.xpos = tr2X;
	trill2.position = [tr2X, height / 2];
}
