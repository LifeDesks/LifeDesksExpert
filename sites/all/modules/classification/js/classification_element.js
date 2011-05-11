$(function() {

  //override the ActiveX jQuery settings
  $.ajaxSetup({
	xhr:function() { return new XMLHttpRequest(); }
  });

  //relocate the tree panel
  var tree_html = $('.classification_tree_panel');
  $('.node-form').parent().css("position","relative").append(tree_html);

  $('.expand').show();
  $('.expand').click(function() {
    $('.classification_tree_div').animate({
      "width": "400px"
	},1500);
    $('.classification_tree_viewer').animate({
      "height" : "500px"
    },1500);
    $(this).hide();
    $('.contract').show();
  });
  $('.contract').click(function() {
    $('.classification_tree_div').animate({
      "width": "230px"
	},500);
	$('.classification_tree_viewer').animate({
	  "height" : "285px"
	},1500);
	$(this).hide();
    $('.expand').show();
  });
});