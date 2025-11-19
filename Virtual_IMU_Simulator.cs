using UnityEngine;
using System.Collections;

public class Virtual_IMU_Simulator : MonoBehaviour
{
    [Header("1. 시뮬레이션 설정")]
    public float rotationSpeed = 60.0f;
    // 시작 시 강제로 100도로 설정됨
    public float fallAngleThreshold = 100.0f; 

    [Header("2. 추락(Free Fall) 설정")]
    public float freeFallTimeThreshold = 1.2f; // 약 7m 낙하 시간
    private float currentFreeFallTimer = 0.0f;

    [Header("3. 충격 효과 설정")]
    public float shockThreshold = 5.0f; 
    public float shakeMagnitude = 0.2f; // 위치 떨림 강도
    public float rotationShakePower = 15.0f; // ★ 회전 떨림 강도 (튕기는 느낌)
    public float shakeDuration = 0.6f;  // 떨리는 시간

    [Header("4. 상태 모니터링")]
    public bool isAccidentDetected = false;
    public string accidentStatus = "정상";
    public float simulatedGForce = 1.0f; 

    private float currentPitch = 0.0f;
    private float currentYaw = 0.0f;
    private float currentRoll = 0.0f;

    private MeshRenderer meshRenderer;
    private Rigidbody rb; 
    private Vector3 initialPosition;
    private Quaternion initialRotation;
    private Color originalColor;
    private bool isFallingSimulated = false;

    void Start()
    {
        // 설정 강제 초기화
        fallAngleThreshold = 100.0f;
        freeFallTimeThreshold = 1.2f;

        meshRenderer = GetComponent<MeshRenderer>();
        rb = GetComponent<Rigidbody>();

        // 시작 각도/위치 저장
        Vector3 currentAngles = transform.eulerAngles;
        currentPitch = currentAngles.x;
        currentYaw = currentAngles.y; 
        currentRoll = currentAngles.z;

        initialPosition = transform.position;
        initialRotation = transform.rotation;

        if (meshRenderer != null) 
        {
            originalColor = meshRenderer.material.color; 
            meshRenderer.material.color = Color.green; 
        }

        // 시작 시 물리 끄기
        if (rb != null)
        {
            rb.useGravity = false;
            rb.isKinematic = true;
        }
        
        Debug.Log($"[시스템 준비] 원래 위치({initialPosition.y}m)에 도달하면 자동으로 멈춥니다.");
    }

    void Update()
    {
        // 1. 추락 중 위치 체크 (가상 바닥 기능)
        if (isFallingSimulated)
        {
            // 현재 높이가 원래 위치(또는 그 이하)로 떨어졌는지 확인
            if (transform.position.y <= initialPosition.y)
            {
                HandleVirtualImpact(); // 착지 처리 함수 호출
            }
        }

        // 2. 키보드 입력 및 회전 (추락 중이 아닐 때만)
        if (!isAccidentDetected && !isFallingSimulated)
        {
            HandleInput();
            transform.rotation = Quaternion.Euler(currentPitch, currentYaw, currentRoll);
        }

        // 3. 센서 데이터 분석
        AnalyzeSensorData();

        // 리셋
        if (Input.GetKeyDown(KeyCode.Space))
        {
            ResetSimulation();
        }
    }

    void HandleInput()
    {
        float dt = Time.deltaTime;

        if (Input.GetKey(KeyCode.W)) currentPitch += rotationSpeed * dt;
        if (Input.GetKey(KeyCode.S)) currentPitch -= rotationSpeed * dt;
        if (Input.GetKey(KeyCode.Q)) currentYaw -= rotationSpeed * dt;
        if (Input.GetKey(KeyCode.E)) currentYaw += rotationSpeed * dt;
        if (Input.GetKey(KeyCode.A)) currentRoll += rotationSpeed * dt;
        if (Input.GetKey(KeyCode.D)) currentRoll -= rotationSpeed * dt;

        // F키: 높은 곳으로 이동 후 추락 시작
        if (Input.GetKeyDown(KeyCode.F)) StartFreeFall();
        
        // K키: 수동 충격
        if (Input.GetKeyDown(KeyCode.K)) 
        {
            TriggerEmergencyAlert("SHOCK (수동 충격)");
            StartCoroutine(ShakeObjectEffect());
        }
    }

    void AnalyzeSensorData()
    {
        if (isAccidentDetected) return;

        // 1. 추락 감지 (0g 유지 시간 체크)
        if (isFallingSimulated) simulatedGForce = 0.0f; 
        else simulatedGForce = 1.0f; 

        if (simulatedGForce < 0.5f)
        {
            currentFreeFallTimer += Time.deltaTime;
            if (currentFreeFallTimer >= freeFallTimeThreshold)
            {
                TriggerEmergencyAlert($"FREE FALL DETECTED ({freeFallTimeThreshold}초 이상 추락)");
            }
        }
        else
        {
            currentFreeFallTimer = 0.0f;
        }

        // 2. 낙상 감지 (100도)
        float angleX = Mathf.DeltaAngle(0, currentPitch); 
        float angleZ = Mathf.DeltaAngle(0, currentRoll);  

        if (Mathf.Abs(angleX) >= fallAngleThreshold || Mathf.Abs(angleZ) >= fallAngleThreshold)
        {
            TriggerEmergencyAlert("FALL DETECTED (넘어짐/낙상)");
        }
    }

    void StartFreeFall()
    {
        if (isAccidentDetected) return;
        if (rb == null) return;

        Debug.LogWarning("!!! 낙하 시작 (원래 위치까지 떨어집니다) !!!");
        
        // 20m 상공으로 이동
        transform.position = new Vector3(initialPosition.x, 20.0f, initialPosition.z);

        isFallingSimulated = true; 
        rb.isKinematic = false; // 물리 켜기
        rb.useGravity = true;   // 중력 켜기
        rb.linearVelocity = Vector3.zero; 
        
        // 떨어질 때도 살짝 회전 추가
        rb.AddTorque(Random.insideUnitSphere * 2.0f); 
    }

    // ★ [핵심 기능] 가상 바닥 충돌 처리
    void HandleVirtualImpact()
    {
        Debug.LogWarning("쿵! 바닥(원래 위치)에 도착했습니다.");

        // 1. 추락 상태 해제 및 물리 정지
        isFallingSimulated = false;
        rb.isKinematic = true; // 물리 끄기 (멈춤)
        rb.useGravity = false;
        rb.linearVelocity = Vector3.zero;

        // 2. 위치를 정확히 원래 높이로 보정
        transform.position = initialPosition;

        // 3. 충격 경고 및 떨림 효과 실행
        TriggerEmergencyAlert("SHOCK (바닥 충돌)");
        StartCoroutine(ShakeObjectEffect());
    }

    // ★ 튕기면서 회전력이 작용하는 떨림 효과
    IEnumerator ShakeObjectEffect()
    {
        float elapsed = 0.0f;
        Vector3 startPos = transform.position;
        Quaternion startRot = transform.rotation; // 충돌 시점의 회전각

        while (elapsed < shakeDuration)
        {
            // 1. 위치 떨림 (진동)
            Vector3 randomPos = startPos + Random.insideUnitSphere * shakeMagnitude;
            transform.position = randomPos;

            // 2. ★ 회전 떨림 (튕기는 느낌)
            // 충돌 시점의 각도에서 무작위로 비틀어 줍니다.
            Quaternion randomRot = Quaternion.Euler(
                Random.Range(-rotationShakePower, rotationShakePower),
                Random.Range(-rotationShakePower, rotationShakePower),
                Random.Range(-rotationShakePower, rotationShakePower)
            );
            transform.rotation = startRot * randomRot;

            elapsed += Time.deltaTime;
            yield return null; 
        }

        // 떨림이 끝나면 위치는 복귀시키되, 회전은 마지막 상태 유지(또는 복귀 선택 가능)
        transform.position = startPos; 
    }

    void TriggerEmergencyAlert(string type)
    {
        if (isAccidentDetected) return; 

        isAccidentDetected = true;
        accidentStatus = type;
        
        if (meshRenderer != null) meshRenderer.material.color = Color.red;

        Debug.LogError($"!!! 긴급 상황 발생: {type} !!!");
        StartCoroutine(TransmitDataToManagerApp(type));
    }

    IEnumerator TransmitDataToManagerApp(string accidentType)
    {
        yield return new WaitForSeconds(1.5f);
        Debug.Log($"[관리자 앱 알림] 🚨 {accidentType} | 위치: {transform.position}");
    }

    void ResetSimulation()
    {
        transform.position = initialPosition;
        transform.rotation = initialRotation;

        Vector3 initialEuler = initialRotation.eulerAngles;
        currentPitch = initialEuler.x;
        currentYaw = initialEuler.y;
        currentRoll = initialEuler.z;

        isAccidentDetected = false;
        accidentStatus = "정상";
        isFallingSimulated = false;
        currentFreeFallTimer = 0.0f;
        simulatedGForce = 1.0f;
        
        if (meshRenderer != null) meshRenderer.material.color = Color.green;

        if (rb != null)
        {
            rb.useGravity = false;
            rb.isKinematic = true;
            rb.linearVelocity = Vector3.zero;
            rb.angularVelocity = Vector3.zero;
        }

        StopAllCoroutines();
        Debug.Log("--- 시스템 리셋 완료 ---");
    }

    void OnDrawGizmos()
    {
        Gizmos.color = Color.black;
        Matrix4x4 rotationMatrix = Matrix4x4.TRS(transform.position, transform.rotation, transform.lossyScale);
        Gizmos.matrix = rotationMatrix;
        Gizmos.DrawWireSphere(Vector3.zero, 0.5f);
    }
}