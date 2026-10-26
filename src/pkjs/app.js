var Clay = require('@rebble/clay');
var config = require('./config');

if (Pebble.getActiveWatchInfo().platform !== 'gabbro') {
  config.forEach(function(item) {
    if (item.messageKey === 'dial') {
      item.options = item.options.filter(function(option) {
        return option.value !== 'perimeter';
      });
    }
  });
}

new Clay(config);
