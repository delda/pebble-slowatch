module.exports = [
  {
    type: 'heading',
    defaultValue: 'SloWatch'
  },
  {
    type: 'radiogroup',
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
    type: 'radiogroup',
    messageKey: 'midnight_position',
    label: 'Midnight position',
    defaultValue: 'bottom',
    options: [
      {
        label: 'Midnight at top',
        value: 'top'
      },
      {
        label: 'Midnight at bottom',
        value: 'bottom'
      }
    ]
  },
  {
    type: 'submit',
    defaultValue: 'Save'
  }
];
