// navigator.bluetooth.requestDevice({
//     filters: [{
//       name: 'Francois robot'
//     }]//,
//     //optionalServices: ['battery_service'] // Required to access service later.
// }).then(device => {
//         // Human-readable name of the device.
//   console.log(device.name);

//   // Attempts to connect to remote GATT Server.
//   return device.gatt.connect();
// }).then(server => { /* … */ }).catch(error => { console.error(error); });


const btn_ = document.querySelector(".btn");

btn_.addEventListener("click", (event) => {
    event.preventDefault();

    console.log("HI");
    navigator.bluetooth.requestDevice({
        acceptAllDevices: true
        // filters: [{
        //     name: 'DRONBOT'
        // }],
        // optionalServices: ['battery_service'] // Required to access service later.
    }).then(device => {
        // Human-readable name of the device.
        console.log(device.name);
        // Attempts to connect to remote GATT Server.
        return device.gatt.connect();
    }).then(server => {
        //TODO
    }).catch(error => {
        console.error(error);
    });
});
