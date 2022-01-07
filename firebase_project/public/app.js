//(function() {
  import  {initializeApp}  from 'firebase/app';
  //import { getDatabase,ref } from 'https://www.gstatic.com/firebasejs/9.6.1/firebase-database.js';

  // TODO: Replace the following with your app's Firebase project configuration
  const firebaseConfig = {
    apiKey: "AIzaSyAcSi97LtykSiVO3VPBLmXA_hj5HXOsLLA",
    authDomain: "espdemofirebase.firebaseapp.com",
    databaseURL: "https://espdemofirebase-default-rtdb.asia-southeast1.firebasedatabase.app",
    projectId: "espdemofirebase",
    storageBucket: "espdemofirebase.appspot.com",
    messagingSenderId: "962438629865",
    appId: "1:962438629865:web:03abe643e5f862d5476378"
  };
  
  // Initialize Firebase
  const app = initializeApp(firebaseConfig);

  const dB=firebase.database().ref();

  const rootRef=dB.ref();
  console.log(rootRef);
//}());