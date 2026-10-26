var Clay = require('@rebble/clay');
var config = require('./config');

var platform = Pebble.getActiveWatchInfo().platform;
if (['gabbro', 'chalk', 'emery', 'basalt'].indexOf(platform) === -1) {
  config.forEach(function(item) {
    if (item.messageKey === 'dial') {
      item.options = item.options.filter(function(option) {
        return option.value !== 'perimeter';
      });
    }
  });
}

new Clay(config);
