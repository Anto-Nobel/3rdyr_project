  import { initializeApp } from "https://www.gstatic.com/firebasejs/9.6.1/firebase-app.js";
            const firebaseConfig = {
                apiKey: "AIzaSyCSSeqZ3GJCPUK14CsNfE9P3I4BJUcQNOQ",
                authDomain: "espsensorjs.firebaseapp.com",
                databaseURL: "https://espsensorjs-default-rtdb.asia-southeast1.firebasedatabase.app",
                projectId: "espsensorjs",
                storageBucket: "espsensorjs.appspot.com",
                messagingSenderId: "563005482371",
                appId: "1:563005482371:web:98ee5791a42d6f63f702b6"
            };
            const app = initializeApp(firebaseConfig);

            import { getDatabase, ref, set, get, child, update, remove, onValue }
                from "https://www.gstatic.com/firebasejs/9.6.1/firebase-database.js"

            const db = getDatabase();
           // To get system date
            let currentDay = new Date().getDate()
            let currentMonth = new Date().getMonth() + 1;
            let currentYear = new Date().getFullYear()

            if (currentMonth == 12) {
                currentMonth = 1;
            }
            if (currentDay < 10) {
                currentDay = `0${currentDay}`;
            }
            if (currentMonth < 10) {
                currentMonth = `0${currentMonth}`;
            }
            let systemCurrentDate = `${currentDay}-${currentMonth}-${currentYear}`;
            //console.log(systemCurrentDate);
           function getAllDataOnce() {
                const dbref = ref(db);
                get(child(dbref, `sensor_1/humidity`))
                    .then((snapshot) => {
                        let humidityDataKey = [];
                        snapshot.forEach(childSnapShot => {
                             let keyValueOfObject = childSnapShot.key;
                            humidityDataKey.push(keyValueOfObject);
                        });
                        console.log(humidityDataKey);
                        humidityDataKey.forEach(childSnapShot =>{
                             get(child(dbref,`sensor_1/humidity/${childSnapShot}`))
                            .then((snapshot)=>{
                            let humidityDataVal = [];
                            humidityDataVal.unshift(childSnapShot)
                            snapshot.forEach(child =>{
                                 humidityDataVal.push(child.val());
                                 
                            });
                            //console.log(humidityDataVal);
                            let averageArray = []
                            let finalAverageReading;
                            let i;
                            for(i=0;i<humidityDataKey.length;i++){
                                if(humidityDataKey[i] == humidityDataVal[0]){
                                    humidityDataVal.shift(humidityDataVal[childSnapShot])
                                     let averageReading = (humidityDataVal.reduce((a, b) => a + b, 0))/humidityDataVal.length;
                                     finalAverageReading = averageReading.toFixed(2)
                                     averageArray.push(finalAverageReading)
                                     console.log(averageArray);
                                } 
                            }
                           
                        })
                        })
                       
                        
                    });
            } window.onload = getAllDataOnce;



