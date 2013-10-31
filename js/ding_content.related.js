/**
 * AJAX routine and UI handling for category foldable menu.
 */
(function ($) {
  Drupal.behaviors.dingContentRelated = {
    attach: function (context, settings) {
      $('.relevant-nodes', context).click(function() {
        var element_settings = {};
        element_settings.url = $(this).attr('href');
        element_settings.progress = { type: 'throbber' };

        if (!$(this).next().hasClass('item-list')) {
          var ajax = new Drupal.ajax(false, false, element_settings);
          ajax.eventResponse(ajax, {});
        }

        if ($(this).hasClass('relevant-nodes-expanded')) {
          $(this).removeClass('relevant-nodes-expanded').siblings().hide();
        }
        else {
          $(this).addClass('relevant-nodes-expanded').siblings().show();
        }

        return false;
      });
    }
  };

  Drupal.ajax.prototype.commands['related_content_insert'] = function (ajax, response, status) {
    var data = $(response.data);
    var tid = response.tid;
    var ele = $('.relevant-nodes-' + tid);

    ele.after(data);

    return;
  };
}(jQuery));
