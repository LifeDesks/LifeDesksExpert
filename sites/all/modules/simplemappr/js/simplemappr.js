var simplemappr_is_inside = false;

$(function() {
    $('body').click(function() {
	  	if(typeof jzoom_api != "undefined" && !simplemappr_is_inside) {
		  jzoom_api.destroy();
		}
    });
    $('input').click(function() {
	  	if(typeof jzoom_api != "undefined" && !simplemappr_is_inside) {
		  jzoom_api.destroy();
		}
    });
   	$('.colorPicker').ColorPicker({
	  onSubmit: function(hsb, hex, rgb, el) {
		$(el).val(rgb.r + ' ' + rgb.g + ' ' + rgb.b);
		$(el).ColorPickerHide();
	  },
	  onBeforeShow: function () {
		$(this).ColorPickerSetColor(this.value);
	  }
    }).bind('keyup', function(){
	  $(this).ColorPickerSetColor(this.value);
    });

	$('#simplemappr-extent').hover(function(){ 
	        simplemappr_is_inside = true; 
	    }, function(){ 
	        simplemappr_is_inside = false; 
	    });

    $('#simplemappr-cropper a').click(function() { 
		initJzoom();
		return false; 
	});

 	function initJzoom(){
		if(typeof jzoom_api != "undefined") {
		  jzoom_api.destroy();
		}
	    jzoom_api = $.Jcrop('#simplemappr-extent img', {
			onChange: __showCoords,
			onSelect: __showCoords
	    });
    }

	function __showCoords(c) {
		var x = parseInt(c.x);
		var y = parseInt(c.y);
		var x2 = parseInt(c.x2);
		var y2 = parseInt(c.y2);
		
		var x_unit = 360/Drupal.settings.simplemappr_extent.x;
		var y_unit = 180/Drupal.settings.simplemappr_extent.y;
		
		$('#edit-simplemappr-user-data-minx').val((-180+x*x_unit).toFixed(2));
		$('#edit-simplemappr-user-data-maxx').val((-180+x2*x_unit).toFixed(2));
		$('#edit-simplemappr-user-data-miny').val((90-y2*y_unit).toFixed(2));
		$('#edit-simplemappr-user-data-maxy').val((90-y*y_unit).toFixed(2));
	}
});