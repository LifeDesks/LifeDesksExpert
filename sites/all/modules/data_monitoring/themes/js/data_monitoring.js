$(document).ready(function() {
  $(".monitor-description").each(function(i) { $(this).addClass("hidden"); });
  $(".monitor-name").each(function(i) { $(this).append("... "); });
  $(".monitor-name").each(function(i) {
    $(this).bind("mouseover focus", function() {
      $(this).addClass("hovering");
      $(this).next(".monitor-description").removeClass("hidden");
    });
    $(this).bind("mouseout blur", function() {
      $(this).next(".monitor-description").addClass("hidden");
      $(this).addClass("hovering");
    });
  });
});
