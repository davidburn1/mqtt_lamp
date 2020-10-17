app.directive('colorWheel', [function () {
    function linkFunc(scope, element, attrs) {
        scope.$watch('hue', function (newVal) {
			colorPicker.color.hsv = { h: scope.hue/65535*360, s: scope.saturation/255*100, v: 100 };
        }, true)


		var colorPicker = new iro.ColorPicker(element[0], {
			width: 320,
			color: "#f00",
			borderWidth: 2,
			borderColor: "#000",
			wheelAngle: 90,

			handleRadius:15,

			layout: [
				{ 
				  component: iro.ui.Wheel,
				  options: {}
				},
			  ]
		  });
		
		scope.clickFn = function(color){
			scope.$apply(function(){
				scope.hue = parseInt(color.hsv['h']/360*65535);
				scope.saturation = parseInt(color.hsv['s']/100*255);
		   });
		}
		colorPicker.on('input:change', scope.clickFn);
    }

    return {
		scope: {'hue':'=', 'saturation':'='},
        link: linkFunc
    };
}]);






// Utility functions (ignore mostly)
function hsv2rgb(h, s, v) {  
    var c = v * s;  
    var h1 = h / 60;  
    var x = c * (1 - Math.abs((h1 % 2) - 1));  
    var m = v - c;  
    var rgb;  
    
    if (typeof h == 'undefined') rgb = [0, 0, 0];
    else if (h1 < 1) rgb = [c, x, 0];
    else if (h1 < 2) rgb = [x, c, 0];
    else if (h1 < 3) rgb = [0, c, x];
    else if (h1 < 4) rgb = [0, x, c];
    else if (h1 < 5) rgb = [x, 0, c];
    else if (h1 <= 6) rgb = [c, 0, x];
    
    return [255 * (rgb[0] + m), 255 * (rgb[1] + m), 255 * (rgb[2] + m)];
} 
function hsv2rgbString(h,s,v){
    var rgb = hsv2rgb(h,s,v);
    rgb = rgb.map(Math.round);
    return "rgb("+rgb[0]+","+rgb[1]+","+rgb[2]+")";
}



var drawPicker = function(){
    context.clearRect (0, 0, 150, 150);
    context.lineWidth = 1;

    for(var i=0;i<totalRadius;i++){
        var radius = totalRadius-i;
        
        if(type == "hsl"){
            var saturation = Math.floor(radius/totalRadius*100)/100;
            var lightness = rangeEl.value;
        }else if(type == "hsv"){
            var saturation = Math.floor(radius/totalRadius*100)/100;
            var value = rangeEl.value;
        }

        for(var angle=0; angle<=360; angle+=1){
            var startAngle = (angle-1) * Math.PI/180;
            var endAngle = (angle+1) * Math.PI/180;
            context.beginPath();
            context.arc(x, y, radius, startAngle, endAngle, false);
            context.closePath();
            if(type == "hsl"){
                context.strokeStyle = 'hsl('+angle+', '+saturation*100+'%, '+lightness*100+'%)';
            }else if(type == "hsv"){
                context.strokeStyle = hsv2rgbString(angle,saturation,value);
            }
            context.stroke();
        }
    }
    
    // Draw the final black border
    context.strokeStyle = "rgb(0,0,0)";
    context.lineWidth = borderWidth;
    context.beginPath();
    var borderRadius = totalRadius+borderWidth/2;
    context.arc(x, y, borderRadius, 0, 2*Math.PI, false);
    context.closePath();
    context.stroke();
}

