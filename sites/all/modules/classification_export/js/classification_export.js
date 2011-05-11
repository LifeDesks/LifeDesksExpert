$(function() {
  TREE = {};
  TREE.show_message = function(message) {
    $('body').append('<div id="classification-overlay"></div><div id="classification-message"></div>');
    var arrPageSizes = ___getPageSize();
    $('#classification-overlay').css({
	  backgroundColor: 'black',
	  opacity: 0.66,
	  width: arrPageSizes[0],
	  height: arrPageSizes[1]
    });
    var arrPageScroll = ___getPageScroll();
    $('#classification-message').css({
	  top: 150,
	  left: arrPageScroll[0],
	  position: 'fixed',
	  zIndex: 1001,
	  margin: '0px auto',
	  width: '100%'
    });
    if(jQuery.ui && jQuery.ui.draggable) {
      $('#classification-message').draggable();
    }
    $('#classification-overlay').show();
    $('#classification-message').html(message).show();
  }
  TREE.hide_message = function() {
	  $('#classification-message').hide();
	  $('#classification-overlay').hide();
  }
  var message = '<div id="classification_warning_message" class="messages-throbber" style="margin:0px auto">';
  message += '<span>' + Drupal.t('Building your classification...') + '</span>';
  message += '<div style="margin-top:10px"><input id="classification_ok" type="submit" class="form-submit" value="' + Drupal.t('Close') + '"></div>';
  message += '</div>';
  $('#edit-submit').click(function() {
	TREE.show_message(message);
	$('#classification_ok').click(function() {
	  TREE.hide_message();
	});
  });
});

function ___getPageSize() {
	var xScroll, yScroll;
	if (window.innerHeight && window.scrollMaxY) {	
		xScroll = window.innerWidth + window.scrollMaxX;
		yScroll = window.innerHeight + window.scrollMaxY;
	} else if (document.body.scrollHeight > document.body.offsetHeight){ // all but Explorer Mac
		xScroll = document.body.scrollWidth;
		yScroll = document.body.scrollHeight;
	} else { // Explorer Mac...would also work in Explorer 6 Strict, Mozilla and Safari
		xScroll = document.body.offsetWidth;
		yScroll = document.body.offsetHeight;
	}
	var windowWidth, windowHeight;
	if (self.innerHeight) {	// all except Explorer
		if(document.documentElement.clientWidth){
			windowWidth = document.documentElement.clientWidth; 
		} else {
			windowWidth = self.innerWidth;
		}
		windowHeight = self.innerHeight;
	} else if (document.documentElement && document.documentElement.clientHeight) { // Explorer 6 Strict Mode
		windowWidth = document.documentElement.clientWidth;
		windowHeight = document.documentElement.clientHeight;
	} else if (document.body) { // other Explorers
		windowWidth = document.body.clientWidth;
		windowHeight = document.body.clientHeight;
	}
	// for small pages with total height less then height of the viewport
	if(yScroll < windowHeight){
		pageHeight = windowHeight;
	} else { 
		pageHeight = yScroll;
	}
	// for small pages with total width less then width of the viewport
	if(xScroll < windowWidth){	
		pageWidth = xScroll;		
	} else {
		pageWidth = windowWidth;
	}
	arrayPageSize = new Array(pageWidth,pageHeight,windowWidth,windowHeight);
	return arrayPageSize;
}

function ___getPageScroll() {
	var xScroll, yScroll;
	if (self.pageYOffset) {
		yScroll = self.pageYOffset;
		xScroll = self.pageXOffset;
	} else if (document.documentElement && document.documentElement.scrollTop) {	 // Explorer 6 Strict
		yScroll = document.documentElement.scrollTop;
		xScroll = document.documentElement.scrollLeft;
	} else if (document.body) {// all other Explorers
		yScroll = document.body.scrollTop;
		xScroll = document.body.scrollLeft;	
	}
	arrayPageScroll = new Array(xScroll,yScroll);
	return arrayPageScroll;
}

$(window).resize(function() {
	// Get page sizes
	var arrPageSizes = ___getPageSize();
	// Style overlay and show it
	$('#classification-overlay').css({
		width: arrPageSizes[0],
		height: arrPageSizes[1]
	});
	var arrPageScroll = ___getPageScroll();
	$('#classification-message').css({
		top: arrPageScroll[1] + (arrPageSizes[3] / 3.5),
		left: arrPageScroll[0],
		position: 'fixed',
		zIndex: 1001,
		margin: '0px auto',
		width: '100%'
	});
});