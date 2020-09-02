let trill1;
let trill2;
let belaLogo;

let timbreDim = [];
let timbreBuffer = [];
let advFmBuffer = [];

let specGraph;
let brigGraph;
let artiGraph;
let enveGraph;
let fftGraph;

let specIn;
let brigIn;
let artiIn;
let enveIn;

let presetMenu;
let presetButton;
let advButton;

let algSelect;
let fRatios = [];
let amps = [];
let shapes = [];
let specButton;
let brightLinkButton;
let brightQSlider;
let artiQSlider;
let dsrSliders = [];
// let susSlider;
// let relSlider;

let NUM_OPS = 4;

// Class Declarations
// Space class to manage Trill displays, dimension inputs, and labels
class Space {
	// inputs are input textboxes
	constructor(spIn, brIn, arIn, enIn) {
		this.x = 0;
		this.y = 0;
		
		this.specIn = spIn;
		this.brigIn = brIn;
		this.artiIn = arIn;
		this.enveIn = enIn;
	}
	
	resize(wid, ht) {
		// width and size of Space
		this.w = 0.71* wid;
		this.h = 0.47 * ht;
		
		// Trill position and size
		this.trillSize = 0.7 * this.h;
		this.trillY = 0.55 * this.h;
		this.trill1X = 0.25 * this.w;
		this.trill2X = 0.75 * this.w;
		
		// label text size
		this.txtSize = 0.029*this.w;
		
		// axis label positions
		this.yAxis1 = 0.58 * this.h;
		this.xAxisLbl1 = 0.25 * this.w;
		this.xAxisLbl2 = 0.75 * this.w;
		this.specLblX = this.trill1X - this.trillSize / 2 - 0.05* this.txtSize;
		this.artiLblX = this.trill2X - this.trillSize / 2 - 0.05* this.txtSize;
		
		// input label positions and sizes
		this.specDisp = 0.17*this.w;
		this.brigDisp = 0.42*this.w;
		this.artiDisp = 0.67*this.w;
		this.enveDisp = 0.92*this.w;
		this.dispW = 0.07*this.w;
		this.dispH = 0.1*this.h;
		this.dispY = 0.05*this.h;
		this.dispLblY = 0.1*this.h;
		
		// input positions and sizes
		this.specIn.size(this.dispW, this.dispH);
		this.brigIn.size(this.dispW, this.dispH);
		this.artiIn.size(this.dispW, this.dispH);
		this.enveIn.size(this.dispW, this.dispH);
		this.specIn.position(0.17*this.w, this.dispY);
		this.brigIn.position(0.42*this.w, this.dispY);
		this.artiIn.position(0.67*this.w, this.dispY);
		this.enveIn.position(0.92*this.w, this.dispY);
	}
	
	draw() {
		// rect space
		// rectMode(CORNER);
		// fill(255);
		// rect(this.x, this.y, this.w, this.h);
		// rect(this.x, this.y, this.w, 0.93*this.h);
		// rect(this.specDisp, this.dispY, 5*this.txtSize, this.txtSize);
		// display
		
		// Draw input labels
		textSize(this.txtSize);
		textAlign(RIGHT, CENTER);
		var colIntens = 127;
		fill(colIntens, 0, 0);
		text('Spectrum ', this.specDisp, this.dispLblY);
		fill(colIntens, colIntens, 0);
		text('Brightness ', this.brigDisp, this.dispLblY);
		fill(0, colIntens, 0);
		text('Articulation ', this.artiDisp, this.dispLblY);
		fill(0, 0, colIntens);
		text('Envelope ', this.enveDisp, this.dispLblY);
		
		// trill axis labels
		textAlign(CENTER, CENTER);
		textSize(this.txtSize);
		fill(colIntens, colIntens, 0);
		text('Brightness', this.trill1X, 0.98*this.h);
		fill(0, 0, colIntens);
		text('Envelope', this.trill2X, 0.98*this.h);
		
		//sideways text (translate, rotate, draw and then undo movements)
		textAlign(CENTER, BOTTOM);
		translate(this.specLblX, this.trillY);
		rotate(-1.57);
		fill(colIntens, 0, 0);
		text('Spectrum', 0, 0);
		rotate(1.57);
		translate(this.artiLblX - this.specLblX, 0);
		rotate(-1.57);
		fill(0, colIntens, 0);
		text('Articulation', 0, 0);
		rotate(1.57);
		translate(-this.artiLblX, -this.trillY);
	}
}

// Graph class managing frame, graph, data, and axis labels
class Graph {
  // inputs are x,y position, size, data, graph color, and axis labels
  constructor(x, y, wid, ht, data, color, xlbl, ylbl) {
    this.resize(x, y, wid, ht);
    this.data = data;
    this.dataType = 0; // how to interpret data
    this.startY = 0;
    this.endY = 0;
    this.color = color;
    this.xLbl = xlbl;
    this.yLbl = ylbl;
  }
  
  setData(data) {
    this.data = data;
  }
  
  setDataType(type) {
  	this.dataType = type;
  }
  
  resize(x, y, wid, ht) {
  	// frame position and size
  	this.x = x;
    this.y = y;
    this.wid = wid;
    this.ht = ht;
    
    // graph position and size
    this.gX = this.x + 0.06*this.wid;
    this.gY = this.y + 0.06*this.ht;
    this.gW = 0.88 * this.wid;
    this.gH = 0.88 * this.ht;
    
    // y-coordinate of origin (bottom left)
    this.y0 = this.gY + this.gH;
    
    //label text sizes
    this.xLblSize = 0.04*this.wid;
    this.yLblSize = 0.04*this.ht;
    
    // positions for axis labels
    this.midX = this.x + 0.5 * this.wid;
    this.midY = this.y + 0.5 * this.ht;
    
  }
  
  // set starting and ending y-coordinates to close the shape
  setEndPoints(start, end) {
  	this.startY = start;
  	this.endY = end;
  }
  
  draw() {
  	// frame
  	stroke(0);
  	rectMode(CORNER);
    fill(255);
    rect(this.x, this.y, this.wid, this.ht, 0.02*this.wid);
    
    // inner graph box
    rect(this.gX, this.gY, this.gW, this.gH);
    
    // axes labels
    fill(0);
    noStroke();
    textSize(this.xLblSize);
    // x axis
    textAlign(CENTER, TOP);
    text(this.xLbl, this.midX, this.gY + this.gH);
    // y axis (sideways text)
    textAlign(CENTER, BOTTOM);
    translate(this.gX, this.midY);
    rotate(-1.57);
    text(this.yLbl, 0, 0);
    rotate(1.57);
    translate(-this.gX, -this.midY);
    stroke(0);
    
    // graph
    fill(this.color);
    var dx;
    var dy;
    var deltaX;
    // create graph using beginShape and closeShape
    beginShape();
    // if type = 1, Treat data as alternating series of x,y,x,y,x,y...
    if (this.dataType == 1) {
    	for (let i = 0; i < this.data.length; i += 2) {
	      dx = this.gW*this.data[i];
	      dy = this.gH*this.data[i + 1];
	      vertex(this.gX + dx, this.y0 - dy);
	    }
	// all other graph types have a start and end point
    } else {
	    // start point
		dy = this.gH * this.startY;
		vertex(this.gX, this.y0 - dy);
		
	    // if the data length is 1, simply plot a horizontal line
	    if (this.data.length == 1) {
    		dy = this.gH*this.data[0];
    		vertex(this.gX, this.y0 - dy);
    		vertex(this.gX + this.gW, this.y0 - dy);
    	
	    // otherwise if type=0, treat data as array of y coords evenly spaced in x
	    } else if (this.dataType === 0) {
	    	// otherwise, plot the data with each element as a y coord
	    	// if the length is 0, then the loop won't be run
		    for (let i = 0; i < this.data.length; i++) {
		      deltaX = i / (this.data.length - 1);
		      dx = this.gW * deltaX;
		      dy = this.gH*this.data[i];
		      vertex(this.gX + dx, this.y0 - dy);
		    }
		// Logarithmic X axis converts linear range [0-22050] and Logarithmic range [10-20000]
	    } else if (this.dataType == 2) {
	    	for (let i = 0; i < this.data.length; i++) {
		      var freq = 22050 * i / (this.data.length);
		      //10-19953
		      deltaX = (Math.log10(freq) - 1) / 3.3;
		      //316-19952
		      //deltaX = (Math.log10(freq) - 2.5) / 1.8;
		      if (deltaX >= 0 && deltaX <= 1) {
			      dx = this.gW * deltaX;
			      dy = this.gH*this.data[i];
			      vertex(this.gX + dx, this.y0 - dy);
		      }
		    }
	    }
	    // end point
		dy = this.gH * this.endY;
	    vertex(this.gX+this.gW, this.y0 - dy);
    }
    // close shape
    endShape(CLOSE);
    noStroke();
  }
}

// block diagram class managing dimension graphs, arrows, and labels
// does not manage final spectrum graph
class BlockDiagram {
	constructor(wid, ht) {
		this.resize(wid, ht);
		
		//can't initialize custom objects within other objects?
		// this.specGraph = new Graph(this.graphSX, this.graphY, this.graphW, this.graphH, [[0,0]]);
		// this.brigGraph = new Graph(this.graphBX, this.graphY, this.graphW, this.graphH, [[0,0]]);
		// this.artiGraph = new Graph(this.graphAX, this.graphY, this.graphW, this.graphH, [[0,0]]);
		// this.enveGraph = new Graph(this.graphEX, this.graphY, this.graphW, this.graphH, [[0,0]]);

	}
	
	resize(wid, ht) {
		this.x = 0;
		this.y = 0.47 * ht;
		this.w = 0.81 * wid;
		this.h = 0.29 * ht;
		
		// graph positions and size
		this.graphH = 0.26 * ht;
		this.graphW = 0.19 * wid;
		this.graphY = 0.485 * ht;
		this.graphSX = 0.01 * wid;
		this.graphBX = 0.21 * wid;
		this.graphAX = 0.41 * wid;
		this.graphEX = 0.61 * wid;
		// this.specGraph.resize(this.graphSX, this.graphY, this.graphW, this.graphH);
		// this.brigGraph.resize(this.graphBX, this.graphY, this.graphW, this.graphH);
		// this.artiGraph.resize(this.graphAX, this.graphY, this.graphW, this.graphH);
		// this.enveGraph.resize(this.graphEX, this.graphY, this.graphW, this.graphH);
		
		// arrows
		this.triSize = 0.02 * ht;
		this.triY = this.graphY + this.graphH / 2;
		this.StBtriX = this.graphSX + this.graphW;
		this.BtAtriX = this.graphBX + this.graphW;
		this.AtEtriX = this.graphAX + this.graphW;
		
		// last arrow
		this.lastTriX = 0.83 * wid;
		this.lastTriTipY = 0.46 * ht;
		this.lastTriBackX = this.graphEX + this.graphW;
		this.lastTriBaseY = 0.48 * ht;
		
		// labels
		this.lblSize = 0.02 * this.w;
		this.lblBracketY = this.y + 0.28 * ht;
		this.lblLineY = this.y + 0.29 * ht;
		this.lblY = this.y + 0.295 * ht;
		this.rawLblX = this.graphSX + this.graphW / 2;
		this.shapeLblX = this.graphAX + this.graphW / 2;
	}
	
	draw() {
		// rectMode(CORNER);
		// fill(255);
		// rect(this.x, this.y, this.w, this.h);
		// this.specGraph.draw();
		// this.brigGraph.draw();
		// this.artiGraph.draw();
		// this.enveGraph.draw();
		
		// draw arrows
		stroke(0);
		strokeWeight(2);
		fill(255);
		// Spectrum to Brightness
		triangle(this.StBtriX, this.triY + this.triSize, this.StBtriX, this.triY - this.triSize, this.graphBX, this.triY);
		// Brightness to Articulation
		triangle(this.BtAtriX, this.triY + this.triSize, this.BtAtriX, this.triY - this.triSize, this.graphAX, this.triY);
		// Articulation to Envelope
		triangle(this.AtEtriX, this.triY + this.triSize, this.AtEtriX, this.triY - this.triSize, this.graphEX, this.triY);
		// last triangle
		triangle(this.lastTriX - this.triSize, this.lastTriBaseY, this.lastTriX + this.triSize, this.lastTriBaseY, this.lastTriX, this.lastTriTipY);
		// tail for envelope-to-final-spectrum arrow
		noFill();
		beginShape();
		vertex(this.lastTriBackX, this.triY);
		vertex(this.lastTriX, this.triY);
		vertex(this.lastTriX, this.lastTriBaseY);
		endShape();
		
		// Labels and brackets to group graphs
		noFill();
		strokeWeight(2);
		stroke(127, 0, 0);
		beginShape();
		vertex(this.graphSX, this.lblBracketY);
		vertex(this.graphSX, this.lblLineY);
		vertex(this.graphSX + this.graphW, this.lblLineY);
		vertex(this.graphSX + this.graphW, this.lblBracketY);
		endShape();
		stroke(0);
		beginShape();
		vertex(this.graphBX, this.lblBracketY);
		vertex(this.graphBX, this.lblLineY);
		vertex(this.graphEX + this.graphW, this.lblLineY);
		vertex(this.graphEX + this.graphW, this.lblBracketY);
		endShape();
		noStroke();
		textSize(this.lblSize);
		textAlign(CENTER, TOP);
		fill(127, 0, 0);
		text('Raw Spectrum', this.rawLblX, this.lblY);
		fill(0);
		text('Shaping Effects', this.shapeLblX, this.lblY);
		stroke(0);
		strokeWeight(1);
	}
}

// Class to manage final spectrum and graph title in top right
class FftDisplay {
	constructor(wid, ht) {
		this.resize(wid, ht);
	}
	resize(wid, ht) {
		this.x = 0.71 * wid;
		this.y = 0;
		this.w = 0.29 * wid;
		this.h = 0.47 * ht;
		
		// graph position and size
		this.graphX = 0.715 * wid;
		this.graphY = 0.1 * ht;
		this.graphW = 0.28 * wid;
		this.graphH = 0.36 * ht;
		
		// title position and size
		this.titleY = 0.09 * ht;
		this.titleSize = 0.029 * wid;
	}
	draw() {
		// rectMode(CORNER);
		// fill(255);
		// rect(this.x, this.y, this.w, this.h);
		
		// graph title
		fill(0);
		noStroke();
		textAlign(LEFT, BOTTOM);
		textSize(this.titleSize);
		text('Final Spectrum', this.graphX, this.titleY);
		stroke(0);
	}
}

// Other controls panel in the bottom left
// manages preset menu, MIDI info, and advanced controls button
class OtherControls {
	// inputs are dropDown menu object, preset button, and advanced controls button
	constructor(dropDown, presetButton, button) {
		this.menu = dropDown;
		this.menu.option(' ');
		this.menu.option('Bass');
		this.menu.option('Piano');
		this.menu.option('Violin');
		this.menu.option('Flute');
		this.menu.option('Clarinet');
		this.menu.option('Timpani');
		this.menu.option('Marimba');
		this.menu.option('Xylophone');
		this.menu.option('Glockenspiel');
		
		this.pButton = presetButton;
		this.aButton = button;
	}
	resize(wid, ht) {
		this.x = 0.81 * wid;
		this.y = 0.47 * ht;
		this.w = 0.19 * wid;
		this.h = 0.53 * ht;
		
		// preset menu and button
		this.presetLblSize = 0.04 * this.w;
		this.presetLblY = this.y + 0.07*this.h;
		this.menu.position(this.x + 0.2*this.w, this.y + 0.095*this.h);
		this.menu.size(0.4*this.w, 0.05*this.h);
		this.pButton.position(this.x + 0.65*this.w, this.y + 0.095*this.h);
		this.pButton.size(0.2*this.w, 0.05*this.h);
		
		// advanced button
		this.aButtonX = this.x + 0.2*this.w;
		this.aButtonY = this.y + 0.6*this.h;
		this.aButton.position(this.aButtonX, this.aButtonY);
		this.aButtonW = 0.6*this.w;
		this.aButtonH = 0.15*this.h;
		this.aButton.size(this.aButtonW, this.aButtonH);
		
		// Midi info
		this.txtSize = 0.1 * this.w;
		this.pitchX = this.x + 0.2*this.w;
		this.pitchY = this.y + 0.3*this.h;
		this.velY = this.y + 0.45*this.h;
	}
	
	// input is a 2-element array of the note and velocity info from Bela
	draw(midiInfo) {
		// rectMode(CORNER);
		// fill(255);
		// rect(this.x, this.y, this.w, this.h);
		
		// preset label
		fill(0);
		noStroke();
		textAlign(LEFT);
		textSize(this.presetLblSize);
		text('Preset Instruments', this.pitchX, this.presetLblY);
		
		
		//MIDI midiInfo
		var note;
		var vel;
		//check conditions to display no numbers
		if (midiInfo === undefined){
			note = "";
			vel = "";
		} else if (midiInfo[0] === 0) {
			note = "";
			vel = "";
		} else {
			note = midiInfo[0];
			vel = midiInfo[1];
		}
		fill(0);
		textAlign(LEFT);
		textSize(this.txtSize);
		text('Note: ' + note, this.pitchX, this.pitchY);
		text('Velocity: ' + vel, this.pitchX, this.velY);
		stroke(0);
		
		// Add BELA logo
		image(belaLogo, this.x + 0.5*this.w, this.y + 0.85*this.h, 0.4*this.w, 0.1*this.h);
	}
}

// Class to manage advaced controls at bottom of screen
class AdvControls {
	// inputs are algorthm dropdown menu, array of frequency ratio input text boxes, array of amplitude input text boxes
	// array of waveshape dropdown menus, update spectrum button, brightness checkbox, brightness slider, articulation slider
	// and array of sliders for decay, sustain, and release
	constructor(algSelectIn, fRatiosIn, ampsIn, shapesIn, specButtonIn, brigButton, brigQSlider, artiQSliderIn, dsrSlidersIn) {
		// algorithm select setup
		this.algSelect = algSelectIn;
		this.algSelect.option('Additive', 0);
		this.algSelect.option('DoubleStack22', 1);
		this.algSelect.option('DoubleStack31', 2);
		this.algSelect.option('DoubleStack33', 3);
		this.algSelect.option('TripleStack1', 4);
		this.algSelect.option('TripleStack2', 5);
		this.algSelect.option('FourStack', 6);
		
		this.fRatios = fRatiosIn;
		this.amps = ampsIn;
		this.shapes = shapesIn;
		this.specButton = specButtonIn;
		// setup options for waveshape menus
		for (let i = 0; i < NUM_OPS; i++) {
			this.shapes[i].option('Sine', 0);
			this.shapes[i].option('Triangle', 1);
			this.shapes[i].option('Square', 2);
			this.shapes[i].option('Saw', 3);
		}
		
		this.brigButton = brigButton;
		this.brigQSlider = brigQSlider;
		this.artiQSlider = artiQSliderIn;
		this.dsrSliders = dsrSlidersIn;
		
		// set advMode to 1 and then toggle it off to hide elements
		this.advMode = 1;
		this.toggleAdvMode();
		
		// arrays of positions for elements
		this.opGridY = [];
		this.algGridX = [];
		this.algGridY = [];
		// array of enables for algorithm display
		this.algEN = [];
		// vertexes for drawing lines of algorithm display
		this.algVertices = [];
		
		//positions for DSR sliders and labels
		this.dsrSlideY = [];
		this.dsrSlideLblY = [];
	}
	resize(wid, ht) {
		this.x = 0;
		this.y = 0.8 * ht;
		this.w = 0.81 * wid;
		this.h = 0.2 * ht;
		
		// pos/size for colored boxes for control areas
		this.specX = 0;
		this.specW = 0.33 * this.w;
		this.brigX = 0.33 * this.w;
		this.brigW = 0.21 * this.w;
		this.artiX = 0.54 * this.w;
		this.artiW = 0.21 * this.w;
		this.enveX = 0.75 * this.w;
		this.enveW = 0.25 * this.w;
		
		//Spectrum FM controls
		rectMode(CORNER);
		this.specButton.position(0.02*this.w, this.y + 0.05*this.h);
		this.specButton.size(0.08*this.w, 0.18*this.h);
		this.algSelect.position(0.02*this.w, this.y + 0.25*this.h);
		this.algSelect.size(0.08*this.w, 30);
		// X-coordinates of algorithm display grid
		for (let i = 0; i < 4; i++)
			this.algGridX[i] = 0.025*(i+1)*this.w;
		var yOffset = this.y + 0.40*this.h;
		// Y-coordinates of algorithm display grid
		for (let i = 0; i < 3; i++)
			this.algGridY[i] = yOffset + 0.15*(i+1)*this.h;
		
		// Positions for operator parameter Grid
		this.gridLblY = this.y + 0.2*this.h;
		this.opColX = 0.13*this.w;
		this.fColX = 0.14*this.w;
		this.ampColX = 0.20*this.w;
		this.waveColX = 0.26*this.w;
		
		// more positions and sizes for operator parameter grid elements
		yOffset = this.y + 0.35*this.h;
		for (let i = 0; i < NUM_OPS; i++) {
			this.opGridY[i] = yOffset + 0.15*i*this.h;
			this.fRatios[i].position(this.fColX, this.opGridY[i]);
			this.amps[i].position(this.ampColX, this.opGridY[i]);
			this.shapes[i].position(this.waveColX, this.opGridY[i]);
			this.fRatios[i].size(0.05*this.w, 0.1*this.h);
			this.amps[i].size(0.05*this.w, 0.1*this.h);
			this.shapes[i].size(0.05*this.w, 0.1*this.h);
		}
		
		// Brightness Controls
		this.brigControlX = 0.36*this.w;
		this.brigSliderY = 0.4*this.h + this.y;
		this.brigSliderLblY = this.brigSliderY - 0.08*this.h;
		this.brigButton.position(this.brigControlX, this.y + 0.05*this.h);
		this.brigQSlider.position(this.brigControlX, this.brigSliderY);
		this.brigQSlider.size(0.15*this.w, 0.1*this.h);
		
		// Articulation Q slider
		this.artiControlX = 0.57*this.w;
		this.artiSliderY = 0.2*this.h + this.y;
		this.artiSliderLblY = this.artiSliderY - 0.08*this.h;
		this.artiQSlider.position(this.artiControlX, this.artiSliderY);
		this.artiQSlider.size(0.15*this.w, 0.1*this.h);
		
		// Envelope dsr sliders
		this.dsrSlideX = 0.76*this.w;
		for (let i = 0; i < 3; i++) {
			this.dsrSlideY[i] = 0.2*(i+1)*this.h + this.y;
			this.dsrSlideLblY[i] = this.dsrSlideY[i] - 0.08*this.h;
			this.dsrSliders[i].position(this.dsrSlideX, this.dsrSlideY[i]);
			this.dsrSliders[i].size(0.2*this.w, 0.1*this.h);
		}
	}
	draw() {
		// rectMode(CORNER);
		// fill(255);
		// rect(this.x, this.y, this.w, this.h);
		
		//only show these elements if we are in advanced mode
		if (this.advMode == 1) {
			// color coded control areas
			var colIntens = 64;
			rectMode(CORNER);
			fill(255, 0, 0, colIntens);
			rect(this.specX, this.y, this.specW, this.h);
			fill(255, 255, 0, colIntens);
			rect(this.brigX, this.y, this.brigW, this.h);
			fill(0, 255, 0, colIntens);
			rect(this.artiX, this.y, this.artiW, this.h);
			fill(0, 0, 255, colIntens);
			rect(this.enveX, this.y, this.enveW, this.h);
			
			// algorithm display is based on a grid of 9 numbers representing operators that can be turned on or off, 
			// depending on the current algorithm. lines signify modulation relationships. For operators connected by
			// lines, the higher operator modulates the lower operator
			// algEN indexes
			// 7 8
			// 4 5 6
			// 0 1 2 3
			// algVertices
			// 0 |-|
			// 1 |-|-|
			// 2 |-|-|-|
			//   0 1 2 3
			var alg = this.algSelect.value();
			// based on algorithm, set enables for numbers and vertices for modulation lines
			switch(alg) {
				// additive
				case '0':
					this.algEN = [1, 1, 1, 1, 0, 0, 0, 0, 0];
					this.algVertices = [];
					break;
				// double stack 22
				case '1':
					this.algEN = [1, 0, 1, 0, 1, 0, 1, 0, 0];
					this.algVertices = [0,2,0,1, 2,2,2,1];
					break;
				// double stack 31
				case '2':
					this.algEN = [1, 1, 1, 0, 0, 0, 1, 0, 0];
					this.algVertices = [2,2,2,1];
					break;
				//double stack 33
				case '3':
					this.algEN = [1, 1, 1, 0, 0, 0, 1, 0, 0];
					this.algVertices = [0,2,2,1, 1,2,2,1, 2,2,2,1];
					break;
				//triple stack 1
				case '4':
					this.algEN = [1, 0, 0, 0, 1, 0, 1, 1, 0];
					this.algVertices = [0,2,0,0, 0,2,2,1];
					break;
				//triple stack 2
				case '5':
					this.algEN = [1, 0, 0, 0, 1, 0, 0, 1, 1];
					this.algVertices = [0,2,0,0, 0,1,1,0];
					break;
				//four stack
				case '6':
					this.algEN = [1, 0, 0, 0, 1, 1, 0, 0, 1];
					this.algVertices = [0,2,0,1, 0,1,1,1, 1,1,1,0];
				
			}
			// algorithm modulation lines
			stroke(160);
			// beginSahape(LINES) will only connect pairs of vertices
			beginShape(LINES);
			for (let i = 0; i < this.algVertices.length; i+=2)
				vertex(this.algGridX[this.algVertices[i]], this.algGridY[this.algVertices[i+1]]);
			endShape();
			noStroke();
			// algorithm operators
			fill(0);
			textAlign(CENTER, CENTER);
			textSize(0.01*this.w);
			var en;
			// you're going to have to trust me that this works and also because it does
			// did it have to be this complicated? probably not.
			for (let i = 0; i < 4; i++) {
				// j limit is i+1 (this is not a full 3x4 grid, j=3 is skipped)
				for (let j = 0; j < i+1; j++) {
					// for bottom row, enable index is i
					if (j === 0) en = i;
					// for all other rows, enable index is calculated as follows
					else en = 2*j + i + 1;
					// check enable array whether or not to show operator number
					if (j != 3 && this.algEN[en] == 1)
						text(i+1, this.algGridX[i-j], this.algGridY[2 - j]);
				}
			}
			
			// Operator Grid Labels
			fill(0);
			textSize(0.015*this.w);
			textAlign(CENTER, TOP);
			text('Op', this.opColX, this.gridLblY);
			for (let i = 0; i < NUM_OPS; i++) {
				text(i+1, this.opColX, this.opGridY[i]);
			}
			textAlign(LEFT, TOP);
			text(' fRatio', this.fColX, this.gridLblY);
			text('Amp', this.ampColX, this.gridLblY);
			text('Wave', this.waveColX, this.gridLblY);
			
			// Brightness Control Labels
			if (this.brigButton.checked())
				this.brigQSlider.hide();
			else {
				this.brigQSlider.show();
				text('Q-factor', this.brigControlX, this.brigSliderLblY);
			}
			// Articulation Control Labels
			text('Q-factor', this.artiControlX, this.artiSliderLblY);
			
			//Envelope Control Labels
			text('Decay', this.dsrSlideX, this.dsrSlideLblY[0]);
			text('Sustain', this.dsrSlideX, this.dsrSlideLblY[1]);
			text('Release', this.dsrSlideX, this.dsrSlideLblY[2]);
		}
		//end of hidden/shown advanced control elements
		
		
		// send advanced mode buffer
		var buffer = [];
		buffer[0] = this.advMode;
		// if checked, brightness is linked to MIDI, send 1
		if (this.brigButton.checked()) buffer[1] = 1;
		else buffer[1] = 0;
		buffer[2] = this.brigQSlider.value();
		buffer[3] = this.artiQSlider.value();
		for (let i = 0; i < 3; i++)
			buffer[i+4] = this.dsrSliders[i].value();
		// always send advanced mode buffer
		// (0th element, advanced mode, will determine if it has any effect)
		// advanced mode buffer index is 1, as determined by initialization order in render.cpp's setup()
		Bela.data.sendBuffer(1, 'float', buffer);
		
		// always send advanced FM Spectrum buffer
		// (will only have an effect if 0th element, the update flag, is 1)
		advFmBuffer[1] = this.algSelect.value();
		for (let i = 0; i < NUM_OPS; i++) {
			advFmBuffer[2 + 3*i] = this.fRatios[i].value();
			advFmBuffer[3 + 3*i] = this.amps[i].value();
			advFmBuffer[4 + 3*i] = this.shapes[i].value();
		}
		// advanced FM buffer index is 2, as determined by initialization order in render.cpp's setup()
		Bela.data.sendBuffer(2, 'float', advFmBuffer);
		// after sending FM buffer, always reset update flag to 0
		advFmBuffer[0] = 0;
		
		// update values of FM display
		// first element of buffer 7 is whether or not to update
		if (Bela.data.buffers[7][0] == 1) {
			// 2nd element of buffer 7 is FM algorithm
			this.algSelect.value(Bela.data.buffers[7][1]);
			for (let i = 0; i < NUM_OPS; i++) {
				this.fRatios[i].value(Bela.data.buffers[8][i].toFixed(2));
				this.amps[i].value(Bela.data.buffers[9][i].toFixed(2));
				this.shapes[i].value(Bela.data.buffers[10][i]);
			}
		}

	}
	
	// toggle advanced mode on the gui by hiding or showing elements
	toggleAdvMode() {
		// if currently in advanced mode, turn off and hide all elements
		if (this.advMode == 1) {
			this.advMode = 0;
			print("toggling off");
			this.algSelect.hide();
			for (let i = 0; i <NUM_OPS; i++) {
				this.fRatios[i].hide();
				this.amps[i].hide();
				this.shapes[i].hide();
			}
			this.brigButton.hide();
			this.brigQSlider.hide();
			this.specButton.hide();
			this.artiQSlider.hide();
			for (let i = 0; i < 3; i++) {
				this.dsrSliders[i].hide();
			}
		}
		// if currently not in advanced mode, turn on and show all elements
		else if (this.advMode === 0) {
			this.advMode = 1;
			print("toggling on");
			this.algSelect.show();
			for (let i = 0; i <NUM_OPS; i++) {
				this.fRatios[i].show();
				this.amps[i].show();
				this.shapes[i].show();
			}
			this.brigButton.show();
			this.brigQSlider.show();
			this.specButton.show();
			this.artiQSlider.show();
			for (let i = 0; i < 3; i++) {
				this.dsrSliders[i].show();
			}
		}
	}
}


// Object declarations
let space;
var blk = new BlockDiagram();
var fft = new FftDisplay();
let other;
let advControls;

// preload function for Trill library and Bela logo
function preload() {
	belaLogo = loadImage('../images/logo_bar14.png');
	loadScript("/libraries/Trill/Trill.js");
}


// At last the setup function, run once at start of code.
function setup() {
	createCanvas(windowWidth, windowHeight);
	
	// Space object
	trill1 = new Trill('square');
	trill2 = new Trill('square');
	specIn = createInput('0');
	brigIn = createInput('0');
	artiIn = createInput('0');
	enveIn = createInput('0');
	specIn.input(updateTimbre);
	brigIn.input(updateTimbre);
	artiIn.input(updateTimbre);
	enveIn.input(updateTimbre);
	space = new Space(specIn, brigIn, artiIn, enveIn);
	
	// block diagram + FFT objects
	specGraph = new Graph(0,0,0,0, [[0,0]], color(255, 0, 0), 'Frequency', 'Amplitude');
	brigGraph = new Graph(0,0,0,0, [[0,0]], color(255, 255, 0), 'Frequency', 'Amplitude');
	artiGraph = new Graph(0,0,0,0, [[0,0]], color(0, 255, 0), 'Time', 'Frequency');
	enveGraph = new Graph(0,0,0,0, [[0,0]], color(0, 0, 255), 'Time', 'Amplitude');
	fftGraph = new Graph(0,0,0,0, [[0,0]],  color(0, 0, 0), 'Frequency', 'Amplitude');
	// specGraph.setDataType(2);
	// brigGraph.setDataType(2);
	enveGraph.setDataType(1);
	// fftGraph.setDataType(2);
	for (let x = 0; x < 4; x++) {
	    timbreDim[x] = 127;
	    timbreBuffer[2*x] = 0;
	    timbreBuffer[2*x+1] = 0;
	}
	
	// other controls object
	presetMenu = createSelect();
	presetButton = createButton('Set');
	presetButton.mousePressed(sendPreset);
	advButton = createButton('Advanced Controls');
	advButton.mousePressed(toggleAdvMode);
	other = new OtherControls(presetMenu, presetButton, advButton);
	
	//advanced controls object
	//FM controls
	algSelect = createSelect();
	for (let i = 0; i < 4; i++)
	{
		fRatios[i] = createInput('1');
		amps[i] = createInput('1');
		shapes[i] = createSelect();
	}
	// update spectrum button
	specButton = createButton('Update');
	specButton.mousePressed(sendSpectrum);
	//Brightness controls
	brightLinkButton = createCheckbox('MIDI Link', true);
	brightQSlider = createSlider(0.71, 2, 1, 0.01);
	// Articulation Q
	artiQSlider = createSlider(0.71, 2, 1, 0.01);
	//decay
	dsrSliders[0] = createSlider(0, 0.3, 0.1, 0.01);
	//sustain
	dsrSliders[1] = createSlider(0, 1, 0.9, 0.01);
	//release
	dsrSliders[2] = createSlider(0, 0.3, 0.1, 0.01);
	advControls = new AdvControls(algSelect, fRatios, amps, shapes, specButton, brightLinkButton, brightQSlider, artiQSlider, dsrSliders);
	
	windowResized();
}

function draw() {
	background(240);
	
	// draw objects
	space.draw();
	blk.draw();
	fft.draw();
	other.draw(Bela.data.buffers[1]);
	advControls.draw();

	// Retrieve data from BELA =========================================
	
	// get trill positions
	// check buffer from BELA if we need to update any timbre
	for (let i = 0; i < 4; i++)
		if (Bela.data.buffers[0][2*i] == 1)
			timbreDim[i] = Bela.data.buffers[0][2*i+1];
	
	// Always send timbre buffer
	// (will only have an effect if 0th element, the update flag, is 1)
	// timbre buffer index is 0, as determined by initialization order in render.cpp's setup()
	Bela.data.sendBuffer(0, 'float', timbreBuffer);
	// always reset all send flags after sending
	for (let i = 0; i < 4; i++)
		if (timbreBuffer[2*i] == 1) {
			timbreDim[i] = timbreBuffer[2*i+1];
			timbreBuffer[2*i] = 0;
		}
	
	// update gui with current timbre dimensions
	draw.touch1Position =  [timbreDim[1] / 255, timbreDim[0] / 255];
	trill1.updateTouch(0, draw.touch1Position, 0.2);
	draw.touch2Position =  [timbreDim[3] / 255, timbreDim[2] / 255];
	trill2.updateTouch(0, draw.touch2Position, 0.2);
	specIn.value(timbreDim[0]);
	brigIn.value(timbreDim[1]);
	artiIn.value(timbreDim[2]);
	enveIn.value(timbreDim[3]);

	// draw graphs
	draw.spfftGraph = Bela.data.buffers[2];
	specGraph.setData(draw.spfftGraph);
	draw.frfGraph = Bela.data.buffers[3];
	brigGraph.setData(draw.frfGraph);
	draw.fcGraph = Bela.data.buffers[4];
	artiGraph.setData(draw.fcGraph);
	draw.adsrGraph = Bela.data.buffers[5];
	enveGraph.setData(draw.adsrGraph);
	draw.outFftGraph = Bela.data.buffers[6];
	fftGraph.setData(draw.outFftGraph);
	
	// special case for articulation graph, if in high pass mode, we want to color above the graph
	// set start and end points at y=1
	if (draw.fcGraph[0] > 0.5  && draw.fcGraph.length != 1)
		artiGraph.setEndPoints(1, 1);
	// otherwise, color below graph, start and end points at y=0
	else 
		artiGraph.setEndPoints(0, 0);
	
	// Draw Reactive Elements =========================================
	trill1.draw();
	trill2.draw();
	
	//draw graphs
	specGraph.draw();
	brigGraph.draw();
	artiGraph.draw();
	enveGraph.draw();
	fftGraph.draw();
	
}

function windowResized() {
	resizeCanvas(windowWidth, windowHeight);
	
	//resize objects
	space.resize(windowWidth, windowHeight);
	blk.resize(windowWidth, windowHeight);
	fft.resize(windowWidth, windowHeight);
	other.resize(windowWidth, windowHeight);
	advControls.resize(windowWidth, windowHeight);
	
	//resize trill objects
	var trWidth = width * 0.35;
	trill1.width = space.trillSize;
	trill1.resize(trill1.width);
	var tr1X = width * 0.07 + trWidth/2;
	trill1.xpos = tr1X;
	trill1.position = [space.trill1X, space.trillY];
	
	trill2.resize(space.trillSize);
	var tr2X = width * 0.93 - trWidth/2;
	trill2.xpos = tr2X;
	trill2.position = [space.trill2X, space.trillY];
	
	// resize graphs
	specGraph.resize(blk.graphSX, blk.graphY, blk.graphW, blk.graphH);
	brigGraph.resize(blk.graphBX, blk.graphY, blk.graphW, blk.graphH);
	artiGraph.resize(blk.graphAX, blk.graphY, blk.graphW, blk.graphH);
	enveGraph.resize(blk.graphEX, blk.graphY, blk.graphW, blk.graphH);
	fftGraph.resize(fft.graphX, fft.graphY, fft.graphW, fft.graphH);
}

// function called to update timbre dimensions with input boxes
function updateTimbre() {
	// retrieve input box values
	var num = [];
	num[0] = Number(specIn.value());
	num[1] = Number(brigIn.value());
	num[2] = Number(artiIn.value());
	num[3] = Number(enveIn.value());
	for (let i = 0; i < 4; i++) {
		var send = 1;
		// convert empty input boxes to 0
		if (num[i] == "") {
			num[i] = 0;
		}
		// if the box matches current value, don't send
		if (num[i] == timbreDim[i]) {
			send = 0;
		}
		// if input box is not a number, don't send
		else if (isNaN(num[i])) {
			send = 0;
			num[i] = 0;
		}
	
		// fill timbre buffer to be sent
		timbreBuffer[2*i] = send;
		timbreBuffer[2*i+1] = num[i]
	}
}

// function called to update timbre dimensions whem preset button is pressed
function sendPreset() {
	var num = [];
	var send = 1;
	switch(presetMenu.value()) {
		// if preset nemu is on blank option do not update any values
		case ' ':
			send = 0;
			num = [0, 0, 0, 0];
			break;
		case 'Bass':
			num = [224, 84, 113, 0];
			break;
		case 'Piano':
			num = [170, 67, 124, 6];
			break;
		case 'Violin':
			num = [161, 123, 108, 253];
			break;
		case 'Flute':
			num = [190, 80, 107, 245];
			break;
		case 'Clarinet':
			num = [126, 90, 111, 206];
			break;
		case 'Timpani':
			num = [36, 123, 143, 20];
			break;
		case 'Marimba':
			num = [26, 124, 113, 0];
			break;
		case 'Xylophone':
			num = [11, 122, 113, 0];
			break;
		case 'Glockenspiel':
			num = [0, 128, 128, 0];
			break;
	}
	// fill timbre buffer to be sent.
	for (let i = 0; i < 4; i++) {
		timbreBuffer[2*i] = send;
		timbreBuffer[2*i+1] = num[i]
	}
}

// wrapper to toggle advanced mode, triggered by the advanced controls button
function toggleAdvMode() {
	advControls.toggleAdvMode();
}

// function called to update spectrum, sets update flag of advanced buffer to high
function sendSpectrum() {
	advFmBuffer[0] = 1;
	// print(algSelect.value());
}

