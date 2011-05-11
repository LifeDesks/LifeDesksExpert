function deleteClassificationWarning(vid) {
	var classification_element = $('#classification-'+vid);
	var classification_name = classification_element.text();
    var message = '<div id="classification_warning_message" class="classification-warning messages error" style="margin:0px auto">';
    message += Drupal.t('Are you sure you want to delete the alternate classification called ' + classification_name + '? All names will be deleted.');
    message += '<div style="margin-top:10px"><input id="edit-delete" type="submit" class="form-submit" value="' + Drupal.t('Delete') + '">';
    message += '  <a id="classification_delete_cancel">' + Drupal.t('Cancel') + '</a>';
    message += '</div></div>';
    var $message = $('#classification-message');
    $('#classification-overlay').fadeIn();
    $message.fadeIn('slow').html(message);
    $('#classification_delete_cancel').click(function() {hide_message();return false;});
    $('#edit-delete').click(function() {deleteClassification(vid);});
}

function hide_message() {
  $('#classification-message').fadeOut();
  $('#classification-overlay').fadeOut();
}

function deleteClassification(vid) {
	var classification_element = $('#classification-'+vid);
	$.post("delete_classification/", {"vid" : vid }, function(data) {displayMessage(data);}, "json");
	classification_element.parents('tr:eq(0)').fadeOut("slow");
}

function displayMessage(content) {
    var $message = $('#classification-message');
    var message = '<div id="classification_warning_message" class="classification-warning messages status" style="margin:0px auto">';
    message += Drupal.t('Classification successfully deleted.') + '<br>';
    message += '<input type="submit" id="classification_ok" class="form-submit" value="' + Drupal.t('OK') + '">';
    message += '</div>';
    $message.html(message);
    $('#classification_ok').click(function() {hide_message(); return false;});
}

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
};

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
};

$(document).ready(function() {

    //set the interface
    $('body').append('<div id="classification-overlay"></div><div id="classification-message"></div>');
    var arrPageSizes = ___getPageSize();
	$('#classification-overlay').css({
		backgroundColor:	'black',
		opacity:			0.66,
		width:				arrPageSizes[0],
		height:				arrPageSizes[1]
	});
	var arrPageScroll = ___getPageScroll();
	$('#classification-message').css({
		top:	arrPageScroll[1] + (arrPageSizes[3] / 3),
		left:	arrPageScroll[0],
		position: 'absolute',
		zIndex: 100,
        margin: '0px auto',
		width: '100%',
	});
});

$(window).resize(function() {
	// Get page sizes
	var arrPageSizes = ___getPageSize();
	// Style overlay and show it
	$('#classification-overlay').css({
		width:		arrPageSizes[0],
		height:		arrPageSizes[1]
	});
	var arrPageScroll = ___getPageScroll();
	$('#classification-message').css({
		top:	arrPageScroll[1] + (arrPageSizes[3] / 3),
		left:	arrPageScroll[0],
		position: 'absolute',
		zIndex: 100,
		margin: '0px auto',
		width: '100%',
	});
});