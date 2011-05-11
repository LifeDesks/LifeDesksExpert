(function($)   {  
     $.fn.vCenter = function() {  
         return this.each(function()  
         {  
             var el = this;  
             $(el).remove().appendTo("body")  
                     .wrap("<div style='position:absolute;top:50%;left:50%;width:1px;height:1px;'></div>")  
                     .css({position: 'relative', left: el.offsetWidth / -2 + "px", top: el.offsetHeight / -2 + "px"});  
         });  
     };  
})(jQuery);


