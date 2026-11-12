module.exports = [
  {
    type: 'heading',
    defaultValue: 'SloWatch'
  },
  {
    type: 'select',
    messageKey: 'dial',
    label: 'Dial',
    defaultValue: 'classic',
    options: [
      { label: 'Classic', value: 'classic' },
      { label: 'Double', value: 'double' },
      { label: 'Perimeter', value: 'perimeter' },
      { label: 'Quarter', value: 'quarter' }
    ]
  },
  {
    type: 'select',
    messageKey: 'color_layout',
    label: 'Color layout',
    defaultValue: 'default',
    options: [
      { label: 'Default — navy / white / copper', value: 'default' },
      { label: 'Snow — white / black / gray', value: 'snow' },
      { label: 'Night — black / white / gray', value: 'night' },
      { label: 'Sand — beige / black / gray', value: 'sand' },
      { label: 'Sky — blue / white / gray', value: 'sky' },
      { label: 'Vanilla — white / black / beige', value: 'vanilla' },
      { label: 'Sun — yellow / black / yellow', value: 'sun' },
      { label: 'Meadow — white / black / green', value: 'meadow' },
      { label: 'Mint — light green / black / gray', value: 'mint' },
      { label: 'Forest — dark green / white / yellow', value: 'forest' }
    ]
  },
  {
    type: 'toggle',
    messageKey: 'midnight_position',
    label: 'Midnight at top',
    description: 'On: midnight at top. Off: midnight at bottom.',
    defaultValue: false
  },
  {
    type: 'submit',
    defaultValue: 'Save'
  }
];
