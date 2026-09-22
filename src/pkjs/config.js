module.exports = [
  {
    type: 'heading',
    defaultValue: 'Settings'
  },
  {
    type: 'radiogroup',
    messageKey: 'midnight_position',
    label: 'Midnight position',
    defaultValue: 'top',
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
