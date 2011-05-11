jQuery.fn.fadeToggle = function(speed, easing, callback) {
  return this.animate({opacity: 'toggle'}, speed, easing, callback);  
};

$(function(){
  var toggled=false;
  $('.classification_tree').click(function() {
    $('#lifedesk_front_page_tree_wrapper').fadeToggle('slow');
    if(toggled==false) {
	  $(this).children('.classification_show').hide();
	  $(this).children('.classification_hidden').show();
	  if($('#classification_tree_block div').length > 0) {
	    $('#classification_tree_viewer').classificationTree();		
	  }
	  toggled = true;
    }
    else {
	  $(this).children('.classification_hidden').hide();
	  $(this).children('.classification_show').show();
	  toggled = false;
    }
  });

  $('.expand').show();
  $('.expand').click(function() {
   $(this).hide();
   $('.contract').show();
   $('#classification_tree_div').animate({
    "width": "400px"
   },1500);
   $('#lifedesk_front_page_tree').animate({
    "width": "410px",
    "height": "535px"
   },1500);
   $('#classification_tree_viewer').animate({
    "height" : "500px"
   },1500);
  });
  $('.contract').click(function() {
   $(this).hide();
   $('.expand').show();
   $('#classification_tree_div').animate({
    "width": "210px"
   },500);
   $('#lifedesk_front_page_tree').animate({
    "width": "220px",
    "height": "325px"
   },500);
   $('#classification_tree_viewer').animate({
    "height" : "290px",
    "min-height" : "290px"
   },500);
  });
});
