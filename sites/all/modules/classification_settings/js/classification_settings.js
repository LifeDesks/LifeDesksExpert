$(function() {
   $('.classification_settings_required').each(function(i) {
	   var _this = this;
	   $(this).change(function() {
		   $('.classification_settings_active:eq(' + i + ')').attr({checked : 'checked', disabled : $(_this).is(':checked')});	
	   });
   });

   //prevent removal of required primary classification from taxon_description form
   $('#edit-classification-settings-taxon-description-req').attr({checked: 'checked', disabled : true});
   $('#edit-classification-settings-taxon-description-wt').attr({disabled : true});
   $('#edit-classification-settings-taxon-description-opt').attr({disabled : true});

   /* Once we have a mechanism to tag names from multiple classifications,
      keep everything below to prevent content types from being selected
   */
   
   jQuery.expr[':'].regex = function(elem, index, match) {
    var matchParams = match[3].split(','),
        validLabels = /^(data|css):/,
        attr = {
            method: matchParams[0].match(validLabels) ? 
                        matchParams[0].split(':')[0] : 'attr',
            property: matchParams.shift().replace(validLabels,'')
        },
        regexFlags = 'ig',
        regex = new RegExp(matchParams.join('').replace(/^\s+|\s+$/g,''), regexFlags);
    return regex.test(jQuery(elem)[attr.method](attr.property));
   }
   $('#edit-vocab').click(function() {
	 if($(this).is(':checked')) {
	   $('#edit-tags').removeAttr('checked').attr("disabled",true);
	   $('#edit-multiple').removeAttr('checked').attr("disabled",true);
	   $('#edit-required').removeAttr('checked').attr("disabled",true);
	   $('#edit-flags').removeAttr('checked').attr("disabled",true);
	   $(':regex(name,^.*nodes.*$)').removeAttr('checked').attr("disabled",true);	
	 }
	 else {
	   $('#edit-tags').removeAttr("disabled");
	   $('#edit-multiple').removeAttr("disabled");
	   $('#edit-required').removeAttr("disabled");
	   $('#edit-weight').removeAttr("disabled");
	   $('#edit-flags').removeAttr("disabled");
	   $(':regex(name,^.*nodes.*$)').removeAttr('disabled');	
	 }
	 
     if($(this).is(':checked') && $(':regex(name,^.*nodes.*$)').is(':checked')) {
	   $(':regex(name,^.*nodes.*$)').removeAttr('checked').attr("disabled",true);
     }	
   });
   $('#edit-flags').click(function() { 
	 if($(this).is(':checked')) {
	   $('#edit-tags').removeAttr('checked').attr("disabled",true);
	   $('#edit-multiple').removeAttr('checked').attr("disabled",true);
	   $('#edit-required').removeAttr('checked').attr("disabled",true);
	   $('#edit-vocab').removeAttr('checked').attr("disabled",true);
	   $(':regex(name,^.*nodes.*$)').removeAttr('checked').attr("disabled",true);	
	 }
	 else {
	   $('#edit-tags').removeAttr("disabled");
	   $('#edit-multiple').removeAttr("disabled");
	   $('#edit-required').removeAttr("disabled");
	   $('#edit-weight').removeAttr("disabled");
	   $('#edit-vocab').removeAttr("disabled");
	   $(':regex(name,^.*nodes.*$)').removeAttr('disabled');	
	 }     
   });
 
   if($('#edit-vocab').is(':checked')) {
	 $('#edit-tags').removeAttr('checked').attr("disabled",true);
	 $('#edit-multiple').removeAttr('checked').attr("disabled",true);
	 $('#edit-required').removeAttr('checked').attr("disabled",true);
	 $('#edit-flags').removeAttr('checked').attr("disabled",true);
     $(':regex(name,^.*nodes.*$)').removeAttr('checked').attr("disabled",true);
   }

   if($('#edit-flags').is(':checked')) {
	 $('#edit-tags').removeAttr('checked').attr("disabled",true);
	 $('#edit-multiple').removeAttr('checked').attr("disabled",true);
	 $('#edit-required').removeAttr('checked').attr("disabled",true);
	 $('#edit-vocab').removeAttr('checked').attr("disabled",true);
   }

   /****************************************/

});