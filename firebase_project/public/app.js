(function() {
  const config = {
  apiKey: "AIzaSyAcSi97LtykSiVO3VPBLmXA_hj5HXOsLLA",
  authDomain: "espdemofirebase.firebaseapp.com",
  databaseURL: "https://espdemofirebase-default-rtdb.asia-southeast1.firebasedatabase.app",
  projectId: "espdemofirebase",
  storageBucket: "espdemofirebase.appspot.com",
  messagingSenderId: "962438629865",
  appId: "1:962438629865:web:03abe643e5f862d5476378"
  };

  firebase.initializeApp(config);
  //const bigTextEvaluationStudents = document.getElementById('temperatureTable');
  const tempTable = document.getElementById('tempTable');
  const rootRef = firebase.database().ref();
  const tempRef=rootRef.child('test1').child('temperature');
  const dateRef=tempRef.equalTo('02-01-2022').orderByKey();

  let tempKey=[];
  let tempVal=[]; 
  dateRef.on('02-01-2022')
  .then(function(snapshot) {
    snapshot.forEach(function(childSnapshot) {
      // key will be "ada" the first time and "alan" the second time
      var tempKeyData = childSnapshot.key;
      tempKey.push(tempKeyData);
      // childData will be the actual contents of the child
      var tempValData = childSnapshot.val();
      tempVal.push(tempValData);
  });
});
  //dbRef.on('value', snap => bigTextEvaluationStudents.innerText = snap.val());

  var table = document.querySelector('#tab1 tbody');
  //const dbEvaluationStudentsRef = firebase.database().ref().child('evaluationStudents');
  dbEvaluationStudentsRef.on('value', snap => {
    while(table.hasChildNodes()) {
		    table.removeChild(table.firstChild);
	  }

    var students = snap.val();
    for(var i in students) {
      var row = table.insertRow(-1);
      for(var j in students[i]) {
				cell = row.insertCell(-1);
				cell.innerHTML = students[i][j];
			}
		}
  });
}());