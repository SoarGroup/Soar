// CONSTANTS
Math.PHI = (1 + Math.sqrt(5)) / 2;

// 3D OBJECTS

function Point(x, y, z) {
    this.type = "Point";
    this.x = x;
    this.y = y;
    this.z = z;
    this.clone = function clone() {
        return new Point(this.x, this.y, this.z);
    };
    // VECTOR
    this.add = function add(p) {
        return new Point(this.x + p.x, this.y + p.y, this.z + p.z);
    };
    this.subtract = function subtract(p) {
        return new Point(this.x - p.x, this.y - p.y, this.z - p.z);
    };
    this.multiply = function multiple(c) {
        return new Point(c * this.x, c * this.y, c * this.z);
    };
    this.divide = function divide(c) {
        return new Point(this.x / c, this.y / c, this.z / c);
    };
    this.dot = function dot(p) {
        return (this.x * p.x + this.y * p.y + this.z * p.z);
    };
    this.cross = function cross(p) {
        return new Point(this.y * p.z - this.z * p.y, this.z * p.x - this.x * p.z, this.x * p.y - this.y * p.x);
    };
    this.length = function length() {
        return Math.sqrt(this.x*this.x + this.y*this.y + this.z*this.z);
    };
    this.normalize = function normalize() {
        var l = this.length();
        return new Point(this.x / l, this.y / l, this.z / l);
    };
    // POINT
    this.draw = function draw(viewport, fillColor, strokeColor, strokeWidth) {
        var context, projected;
        context = viewport.context;
        context.fillStyle = fillColor;
        context.strokeStyle = strokeColor;
        context.lineWidth = strokeWidth;
        projected = viewport.project(this);
        context.beginPath();
        context.arc(projected.x, projected.y, 3, 0, 2 * Math.PI, true);
        context.fill();
        context.stroke();
        context.closePath();
    };
    this.rotate = function rotate(theta, phi) {
        var sinTheta, cosTheta, sinPhi, cosPhi, newX, newY, newZ;
        sinTheta = Math.sin(theta);
        cosTheta = Math.cos(theta);
        sinPhi = Math.sin(phi);
        cosPhi = Math.cos(phi);
        newX = this.x * cosTheta + this.z * sinTheta;
        newY = this.x * sinTheta * sinPhi;
        newY += this.y * cosPhi;
        newY -= this.z * cosTheta * sinPhi;
        newZ = -this.x * sinTheta * cosPhi;
        newZ += this.y * sinTheta;
        newZ += this.z * cosTheta * cosPhi;
        this.x = newX;
        this.y = newY;
        this.z = newZ;
        return this;
    };

    this.toString = function toString() {
        return "<" + this.x + ", " + this.y + ", " + this.z + ">";
    };
    this.translate = function translate(vector) {
        this.x += vector.x;
        this.y += vector.y;
        this.z += vector.z;
        return this;
    };
}

function Line(point1, point2) {
    this.type = "Line";
    this.point1 = point1;
    this.point2 = point2;
    this.clone = function clone() {
        return new Line(this.point1.clone(), this.point2.clone());
    };
    this.draw = function draw(viewport, fillColor, strokeColor, strokeWidth) {
        var context, projected1, projected2;
        context = viewport.context;
        context.fillStyle = fillColor;
        context.strokeStyle = strokeColor;
        context.lineWidth = strokeWidth;
        projected1 = viewport.project(this.point1);
        projected2 = viewport.project(this.point2);
        context.beginPath();
        context.moveTo(projected1.x, projected1.y);
        context.lineTo(projected2.x, projected2.y);
        context.stroke();
        context.closePath();
    };
    this.rotate = function rotate(theta, phi) {
        this.point1.rotate(theta, phi);
        this.point2.rotate(theta, phi);
        return this;
    };
    this.toString = function toString() {
        return this.point1.toString() + " -- " + this.point2.toString();
    };
    this.translate = function translate(vector) {
        this.point1.translate(vector);
        this.point2.translate(vector);
        return this;
    };
}

function Label(coord, text) {
    this.type = "Label";
    this.coord = coord;
    this.text = text;
    this.clone = function clone() {
        return new Label(this.coord.clone(), this.text);
    };
    this.draw = function draw(viewport, fillColor, strokeColor, strokeWidth) {
        var context, projected;
        context = viewport.context;
        context.fillStyle = fillColor;
        context.strokeStyle = strokeColor;
        context.lineWidth = strokeWidth;
        projected = viewport.project(this.coord);
        context.fillStyle = fillColor;
        context.strokeStyle = strokeColor;
        context.labelWidth = strokeWidth;
        context.fillText(this.text, projected.x, projected.y);
    };
    this.rotate = function rotate(theta, phi) {
        this.coord.rotate(theta, phi);
        return this;
    };
    this.toString = function toString() {
        return this.coord.toString() + " " + this.text;
    };
    this.translate = function translate(vector) {
        this.coord.translate(vector);
        return this;
    };
} 

// VIEWPORT

function Viewport(canvasID) {
    // VARIABLES
    this.canvas = document.getElementById(canvasID);
    this.context = this.canvas.getContext("2d");
    this.width = this.canvas.width;
    this.height = this.canvas.height;
    this.theta = 1 * Math.PI / 4;
    this.phi = Math.PI / 4;
    this.scale = 75;
    this.displayOffset = {x:this.width / 2, y:this.height / 2};
    this.origin = new Point(0, 0, 0);
    this.changed = false;
    // MATH
    this.project = function project(point) {
        var projected = point.subtract(this.origin);
        projected.rotate(this.theta, this.phi);
        projected.x = projected.x * this.scale + this.displayOffset.x;
        projected.y = -projected.y * this.scale + this.displayOffset.y;
        projected.z = 0;
        return projected;
    };
    // MOVEMENT
    this.moveOffset = function moveOffset(x, y) {
        if (isNaN(x) || isNaN(y)) {
            return;
        }
        this.displayOffset.x = Math.round(this.displayOffset.x + x);
        this.displayOffset.y = Math.round(this.displayOffset.y + y);
        this.changed = true;
    };
    this.moveCamera = function moveCamera(theta, phi) {
        this.theta += theta;
        this.phi += phi;
        if (this.theta > 2 * Math.PI) {
            this.theta -= 2 * Math.PI;
        } else if (this.theta < 0) {
            this.theta += 2 * Math.PI;
        }
        this.phi = Math.max(Math.min(this.phi, Math.PI / 2), -Math.PI / 2);
        this.changed = true;
    };
    this.reset = function reset() {
        this.width = this.canvas.width;
        this.height = this.canvas.height;
        this.theta = 7 * Math.PI / 4;
        this.phi = Math.PI / 4;
        this.scale = 1;
        this.displayOffset = {x:this.width / 2, y:this.height / 2};
        this.origin = new Point(0, 0, 0);
        this.changed = true;
    };
    this.zoom = function zoom(levels) {
        this.scale *= Math.pow(Math.PHI, levels);
        this.changed = true;
    };
    // DRAWING
    this.clear = function clear() {
        this.context.fillStyle = "rgb(255,255,255)";
        this.context.fillRect(0, 0, this.width, this.height);
    };
    this.drawAxes = function drawAxes() {
        var x, y, z, axisLength, xAxis, yAxis, zAxis;
        x = this.origin.x;
        y = this.origin.y;
        z = this.origin.z;
        axisLength = Math.min(this.width, this.height) / (2 * this.scale);
        axisLength = 3 * axisLength / 4;
        xAxis = new Line(new Point(x, y, z), new Point(x + axisLength, y, z));
        yAxis = new Line(new Point(x, y, z), new Point(x, y + axisLength, z));
        zAxis = new Line(new Point(x, y, z), new Point(x, y, z + axisLength));
        xAxis.draw(this, "rgb(0,0,0)", "rgb(0,0,0)", 1);
        yAxis.draw(this, "rgb(0,0,0)", "rgb(0,0,0)", 1);
        zAxis.draw(this, "rgb(0,0,0)", "rgb(0,0,0)", 1);
        new Label(xAxis.point2, "x").draw(this, "rgb(0,0,0)", "rgb(0,0,0)", 1);
        new Label(yAxis.point2, "y").draw(this, "rgb(0,0,0)", "rgb(0,0,0)", 1);
        new Label(zAxis.point2, "z").draw(this, "rgb(0,0,0)", "rgb(0,0,0)", 1);
    };
}

// INTERACTION

function createInteractiveViewport(canvasID, callback) {
    var viewport, mouseDownHandler, mouseMoveHandler, mouseUpHandler, mouseWheelHandler, keyDownHandler, drawUpdate, canvas, button, dragging, lastPos;
    viewport = new Viewport(canvasID);
    button = null;
    dragging = false;
    lastPos = null;
    mouseDownHandler = function mouseDownHandler(e) {
        button = e.button;
        dragging = true;
        lastPos = {x:e.pageX - e.target.offsetLeft, y:e.pageY - e.target.offsetTop};
    };
    mouseMoveHandler = function moveMoveHandler(e) {
        var x, y;
        if (!dragging) {
            return;
        }
        x = e.pageX - e.target.offsetLeft;
        y = e.pageY - e.target.offsetTop;
        if (button === 0) { // left click
            viewport.moveCamera((x - lastPos.x) / 15, (y - lastPos.y) / 15);
        } else if (button === 1) { // middle click
            viewport.moveOffset(x - lastPos.x, y - lastPos.y);
        }
        lastPos = {x:x, y:y};
    };
    mouseUpHandler = function mouseUpHandler(e) {
        dragging = false;
        lastPos = null;
    };
    mouseWheelHandler = function mouseWheelHandler(e) {
        var levels;
        if (e.wheelDelta) {
            levels = e.wheelDelta / 120;
        } else if (e.detail) {
            levels = -e.detail / 3;
        }
        viewport.zoom(levels);
    };
    keyDownHandler = function keyDownHander(e) {
        var keyCode = e.keyCode;
        switch(keyCode) {
            case 36: // home
                viewport.reset();
                break;
            case 37: // left
                viewport.moveCamera(-1/15, 0);
                break; 
            case 38: // up
                viewport.moveCamera(0, 1/15);
                break; 
            case 39: // right
                viewport.moveCamera(1/15, 0);
                break; 
            case 40: // down
                viewport.moveCamera(0, -1/15);
                break; 
        }
    };
    drawUpdate = function draw(callback, viewport) {
        callback(viewport);
        viewport.changed = false;
    };
    canvas = viewport.canvas;
    canvas.addEventListener("mousedown", mouseDownHandler, false);
    canvas.addEventListener("mousemove", mouseMoveHandler, false);
    canvas.addEventListener("mouseup", mouseUpHandler, false);
    canvas.addEventListener("DOMMouseScroll", mouseWheelHandler, false);
    canvas.addEventListener("mousewheel", mouseWheelHandler, false);
    document.addEventListener("keydown", keyDownHandler, false);
	callback(viewport);
    setInterval(function() { drawUpdate(callback, viewport); }, 100);
	this.changed=true;
	this.mydodec = null;
	setInterval(function() { httpRequest( 'get', "http://localhost:8000/scene.out", true, handle ); }, 1000);
    return viewport;
}

// TESTING

function drawDodec(viewport) {
    if (!(this.changed || viewport.changed)) {
        return;
    }
    viewport.clear();
	if (this.mydodec==null)
	{
		return;
	}
    for (var i in this.mydodec) {
        this.mydodec[i][0].draw(viewport, this.mydodec[i][1], this.mydodec[i][1], 1);
    }
}

function createDodecahedron(txt) {
    
	var colors = [
		"Black",
		"DarkGreen",
		"Maroon",
		"Coral",
		"Navy",
		"Purple",
		"Brown",
		"DarkCyan",
		"DimGray",
	];
	
	this.mydodec = [];
	var func = new Function("return " + txt);
	var json = func();
	
	var ptStart = 0;
	var ptEnd = 0;
	for (var obj in json)
	{
		for (var pt in json[obj])
		{
			this.mydodec.push([new Point(json[obj][pt][0], json[obj][pt][1], json[obj][pt][2]), colors[obj]]);
		}
		
		ptEnd = this.mydodec.length;
		
		for (var i=ptStart; i<ptEnd; i++)
		{
			for (var j=(i+1); j<ptEnd; j++)
			{
				if (i!=j)
				{
					this.mydodec.push([new Line(this.mydodec[i][0], this.mydodec[j][0]),colors[obj]]);
				}
			}
		}
		
		ptStart = this.mydodec.length;
	}
}

function send()
{
    httpRequest( 'get', url, true, handle );
}

function handle()
{
	if (request.readyState == 4)
    {
        if (request.status == 200)
        {            
            createDodecahedron(request.responseText);
        }
        else
        {
            throw new Error("scene file not found");
        }
    }
}
