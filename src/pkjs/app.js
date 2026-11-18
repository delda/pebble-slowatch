var Clay = require('@rebble/clay');
var config = require('./config');

config[1].options.push({ label: 'Gold Ring', value: 'gold_ring' });

var clay = new Clay(config);

// Preserve the position saved by the previous radio control.
try {
  var settings = JSON.parse(localStorage.getItem('clay-settings')) || {};
  if (settings.midnight_position === 'top' || settings.midnight_position === 'bottom') {
    clay.setSettings('midnight_position', settings.midnight_position === 'top');
  }
} catch (error) {
  console.error('Unable to migrate midnight position: ' + error);
}
