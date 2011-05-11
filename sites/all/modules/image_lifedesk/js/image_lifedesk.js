$(function(){
	var options = {
		    zoomWidth: 290,
		    zoomHeight: 250,
	        xOffset: 10,
	        yOffset: 0,
	        position: "right",
	        title: false,
			preloadText: Drupal.t('Loading zoom')
	};
	$('.jqzoom').jqzoom(options);
});